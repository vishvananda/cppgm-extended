#include "syntax/ast_clone.h"

#include <algorithm>
#include <new>

#include "support/small_list.h"

namespace syntax {

namespace {

// A template argument that is a pack expansion: marked so, or a type-id
// whose declarator has the pack's ellipsis (`T...`).
bool IsPackArgument(const TemplateArgument& argument)
{
	return argument.pack ||
		(argument.type != nullptr && argument.type->declarator != nullptr &&
			argument.type->declarator->pack != PackPosition::None);
}

} // namespace

template<typename T>
T* SyntaxCloner::copy(const T* node)
{
	return new (arena_.allocate(sizeof(T), alignof(T))) T(*node);
}

template<typename T>
Slice<T*> SyntaxCloner::pointers(const Slice<T*>& list, T* (SyntaxCloner::*each)(const T*))
{
	T** out = nullptr;
	for (uint32_t i = 0; i < list.size; ++i)
	{
		T* item = (this->*each)(list[i]);
		if (item != list[i] && out == nullptr)
		{
			out = static_cast<T**>(arena_.allocate(sizeof(T*) * list.size, alignof(T*)));
			for (uint32_t j = 0; j < i; ++j)
				out[j] = list[j];
		}
		if (out != nullptr)
			out[i] = item;
	}
	return out == nullptr ? list : Slice<T*>{ out, list.size };
}

Slice<Expr*> SyntaxCloner::exprs(const Slice<Expr*>& list)
{
	// Every expression is copied (each carries facts).
	if (list.size == 0)
		return list;
	if (expander_ != nullptr && collecting_ == nullptr)
	{
		for (const Expr* item : list)
		{
			if (item->kind == ExprKind::PackExpansion)
				return expanded_exprs(list);
		}
	}
	Expr** out = static_cast<Expr**>(arena_.allocate(sizeof(Expr*) * list.size, alignof(Expr*)));
	for (uint32_t i = 0; i < list.size; ++i)
		out[i] = expr(list[i]);
	return Slice<Expr*>{ out, list.size };
}

// ------------------------------------------------------------------ names

bool SyntaxCloner::component(const NameComponent& in, NameComponent& out, bool leading)
{
	out = in;
	bool changed = false;
	if (expander_ != nullptr && leading && in.kind == NameComponentKind::Identifier)
	{
		if (collecting_ != nullptr)
		{
			if (length(in.identifier) != -1 &&
				std::find(collecting_->begin(), collecting_->end(), in.identifier) == collecting_->end())
				collecting_->push_back(in.identifier);
		}
		else if (const Identifier* element = renamed(in.identifier))
		{
			out.identifier = element;
			changed = true;
		}
	}
	if (in.kind == NameComponentKind::Conversion)
	{
		out.conversion = type_id(in.conversion);
		changed = changed || out.conversion != in.conversion;
	}
	else if (in.kind == NameComponentKind::Decltype ||
		(in.kind == NameComponentKind::Destructor && in.destructor_decltype))
	{
		out.decltype_operand = expr(in.decltype_operand);
		changed = true;
	}
	if (in.arguments.size == 0)
		return changed;
	if (expander_ != nullptr && collecting_ == nullptr)
	{
		for (const TemplateArgument& argument : in.arguments)
		{
			if (IsPackArgument(argument))
			{
				out.arguments = expanded_arguments(in.arguments);
				out.expanded = true;
				return true;
			}
		}
	}
	TemplateArgument* arguments = static_cast<TemplateArgument*>(
		arena_.allocate(sizeof(TemplateArgument) * in.arguments.size, alignof(TemplateArgument)));
	bool arguments_changed = false;
	for (uint32_t i = 0; i < in.arguments.size; ++i)
	{
		arguments[i] = in.arguments[i];
		// A nested pack expansion expands its own packs.
		if (collecting_ != nullptr && IsPackArgument(in.arguments[i]))
		{
			nested_expansion([&]() {
				type_id(in.arguments[i].type);
				expr(in.arguments[i].expression);
			});
			continue;
		}
		arguments[i].type = type_id(in.arguments[i].type);
		arguments[i].expression = expr(in.arguments[i].expression);
		arguments_changed = arguments_changed || arguments[i].type != in.arguments[i].type ||
			arguments[i].expression != nullptr;
	}
	if (arguments_changed)
		out.arguments = Slice<TemplateArgument>{ arguments, in.arguments.size };
	return changed || arguments_changed;
}

Name* SyntaxCloner::name(const Name* name, bool leading)
{
	if (name == nullptr)
		return nullptr;
	NameComponent* components = nullptr;
	for (uint32_t i = 0; i < name->components.size; ++i)
	{
		NameComponent item;
		const bool changed = component(name->components[i], item, leading && i == 0 && !name->global);
		if (changed && components == nullptr)
		{
			components = static_cast<NameComponent*>(
				arena_.allocate(sizeof(NameComponent) * name->components.size, alignof(NameComponent)));
			for (uint32_t j = 0; j < i; ++j)
				components[j] = name->components[j];
		}
		if (components != nullptr)
			components[i] = item;
	}
	if (components == nullptr)
		return const_cast<Name*>(name);
	Name* result = copy(name);
	result->components = Slice<NameComponent>{ components, name->components.size };
	return result;
}

// --------------------------------------------------------- pack expansion

int32_t SyntaxCloner::length(const Identifier* name) const
{
	// -1 for no pack, -2 for a pack that a template declared in the copy
	// binds (see shadows_).
	for (size_t i = shadows_.size(); i-- > 0;)
	{
		if (shadows_[i].first == name)
			return shadows_[i].second;
	}
	for (const std::pair<const Identifier*, uint32_t>& pack : parameter_packs_)
	{
		if (pack.first == name)
			return static_cast<int32_t>(pack.second);
	}
	return expander_->pack_length(name);
}

const Identifier* SyntaxCloner::renamed(const Identifier* name) const
{
	for (const std::pair<const Identifier*, const Identifier*>& rename : renames_)
	{
		if (rename.first == name)
			return rename.second;
	}
	return nullptr;
}

template<typename Copy>
int32_t SyntaxCloner::pattern_packs(Copy copy_pattern, std::vector<const Identifier*>& packs, uint32_t location)
{
	// The packs a pattern names outside the expansions nested in it (each
	// of which expands its own, 14.5.3p5), found by a copy made only for
	// that and released at once (the instantiation's expander looks names
	// up in its own table, allocating nothing); their common length, or -1
	// when it names none, or a pack of a template declared in the copy
	// (whose instantiations expand it).
	std::vector<const Identifier*>* saved = collecting_;
	std::vector<std::pair<const Identifier*, const Identifier*>> saved_renames;
	saved_renames.swap(renames_);
	collecting_ = &packs;
	const Arena::Mark mark = arena_.mark();
	copy_pattern();
	arena_.rewind(mark);
	collecting_ = saved;
	renames_.swap(saved_renames);
	if (packs.empty())
		return -1;
	for (const Identifier* pack : packs)
	{
		if (length(pack) < 0)
			return -1;
	}
	const int32_t count = length(packs[0]);
	for (const Identifier* pack : packs)
	{
		if (length(pack) != count)
			expander_->mismatch(location);
	}
	return count;
}

void SyntaxCloner::shadow(const Declarator* declarator, int32_t length)
{
	// A declarator's identifier, when it is a pack's name here (or declares
	// a template's pack).
	if (expander_ == nullptr)
		return;
	for (; declarator != nullptr; declarator = declarator->nested)
	{
		if (declarator->name == nullptr || declarator->name->qualified() ||
			declarator->name->last().kind != NameComponentKind::Identifier)
			continue;
		const Identifier* id = declarator->name->last().identifier;
		if (length == -2 || this->length(id) >= 0)
			shadows_.push_back(std::make_pair(id, length));
		return;
	}
}

void SyntaxCloner::shadow_template_packs(const TemplateParameterList* list)
{
	for (uint32_t i = 0; expander_ != nullptr && list != nullptr && i < list->parameters.size; ++i)
	{
		const TemplateParameter* parameter = list->parameters[i];
		if (!parameter->pack)
			continue;
		if (parameter->kind == TemplateParameterKind::NonType)
			shadow(parameter->declarator, -2);
		else if (parameter->name != nullptr)
			shadows_.push_back(std::make_pair(parameter->name, -2));
	}
}

void SyntaxCloner::collect_packs(
	const Expr* expr, std::vector<const Identifier*>& out, std::vector<const Identifier*>* nested)
{
	std::vector<const Identifier*>* saved = collecting_;
	collecting_ = &out;
	nested_ = nested;
	expansions_ = pack_sizes_ = this_uses_ = 0;
	this->expr(expr);
	collecting_ = saved;
	nested_ = nullptr;
}

template<typename Walk>
void SyntaxCloner::nested_expansion(Walk walk)
{
	// An expansion nested in the expression whose packs are collected:
	// counted, and its pattern's packs (with those of the expansions in
	// it) collected apart when they are asked for.
	++expansions_;
	if (nested_ == nullptr)
		return;
	std::vector<const Identifier*>* saved = collecting_;
	collecting_ = nested_;
	walk();
	collecting_ = saved;
}

Expr* SyntaxCloner::element_expr(
	const Expr* expr, const std::vector<std::pair<const Identifier*, const Identifier*>>& renames)
{
	renames_ = renames;
	Expr* result = this->expr(expr);
	renames_.clear();
	return result;
}

void SyntaxCloner::rename_to(const std::vector<const Identifier*>& packs, uint32_t index)
{
	renames_.clear();
	for (const Identifier* pack : packs)
		renames_.push_back(std::make_pair(pack, expander_->element_name(pack, index)));
}

Slice<Expr*> SyntaxCloner::expanded_exprs(const Slice<Expr*>& list)
{
	// An expression list whose pack expansions (14.5.3p4) become one copy
	// of their pattern per element.
	std::vector<Expr*> out;
	for (Expr* item : list)
	{
		if (item->kind != ExprKind::PackExpansion)
		{
			out.push_back(expr(item));
			continue;
		}
		const Expr* pattern = static_cast<const PackExpansionExpr*>(item)->pattern;
		std::vector<const Identifier*> packs;
		const int32_t count = pattern_packs([&]() { expr(pattern); }, packs, item->location);
		if (count < 0)
		{
			out.push_back(expr(item));
			continue;
		}
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		saved.swap(renames_);
		for (int32_t i = 0; i < count; ++i)
		{
			rename_to(packs, static_cast<uint32_t>(i));
			out.push_back(expr(pattern));
		}
		renames_.swap(saved);
	}
	if (out.empty())
		return Slice<Expr*>{ nullptr, 0 };
	return Slice<Expr*>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

Expr* SyntaxCloner::expanded_fold(const FoldExpr* fold)
{
	// C++17's 5.1.6p9: the operator applied over the pattern's copies, one
	// per pack element -- E1 op (... op (En op I)) for a right fold, (((I
	// op E1) op E2) ...) op En for a left one, the init (a binary fold's)
	// innermost; an empty unary &&, || or comma fold is true, false or
	// void() ([temp.variadic]p9), any other ill-formed. The pattern
	// is the operand naming a pack (a binary fold's either); null when no
	// pack is known yet.
	const Expr* pattern = fold->pattern;
	const Expr* init = fold->init;
	bool right = fold->right;
	std::vector<const Identifier*> packs;
	int32_t count = pattern_packs([&]() { this->expr(pattern); }, packs, fold->location);
	if (count < 0 && init != nullptr)
	{
		packs.clear();
		count = pattern_packs([&]() { this->expr(init); }, packs, fold->location);
		if (count >= 0)
		{
			std::swap(pattern, init);
			right = false;
		}
	}
	if (count < 0)
		return nullptr;
	std::vector<Expr*> elements;
	std::vector<std::pair<const Identifier*, const Identifier*>> saved;
	saved.swap(renames_);
	for (int32_t i = 0; i < count; ++i)
	{
		rename_to(packs, static_cast<uint32_t>(i));
		elements.push_back(this->expr(pattern));
	}
	renames_.swap(saved);
	Expr* accumulated = init != nullptr ? this->expr(init) : nullptr;
	if (accumulated == nullptr && elements.empty())
	{
		if (fold->op == TokenKind::OP_COMMA)
		{
			DeclSpecifier* specifier = static_cast<DeclSpecifier*>(
				arena_.allocate(sizeof(DeclSpecifier), alignof(DeclSpecifier)));
			*specifier = DeclSpecifier();
			specifier->kind = DeclSpecKind::Keyword;
			specifier->keyword = TokenKind::KW_VOID;
			specifier->location = fold->location;
			FunctionalCastExpr* cast = static_cast<FunctionalCastExpr*>(
				arena_.allocate(sizeof(FunctionalCastExpr), alignof(FunctionalCastExpr)));
			*cast = FunctionalCastExpr();
			cast->kind = ExprKind::FunctionalCast;
			cast->location = fold->location;
			cast->type = Slice<DeclSpecifier>{ specifier, 1 };
			cast->arguments = Slice<Expr*>{ nullptr, 0 };
			cast->braced = nullptr;
			return cast;
		}
		if (fold->op != TokenKind::OP_LAND && fold->op != TokenKind::OP_LOR)
			return nullptr;
		KeywordLiteralExpr* literal = static_cast<KeywordLiteralExpr*>(
			arena_.allocate(sizeof(KeywordLiteralExpr), alignof(KeywordLiteralExpr)));
		*literal = KeywordLiteralExpr();
		literal->kind = ExprKind::KeywordLiteral;
		literal->location = fold->location;
		literal->keyword = fold->op == TokenKind::OP_LAND ? TokenKind::KW_TRUE : TokenKind::KW_FALSE;
		return literal;
	}
	const bool assignment = fold->op != TokenKind::OP_COMMA && IsAssignmentOperator(fold->op);
	const auto apply = [&](Expr* left, Expr* right_operand) {
		BinaryExpr* node = static_cast<BinaryExpr*>(arena_.allocate(sizeof(BinaryExpr), alignof(BinaryExpr)));
		*node = BinaryExpr();
		node->kind = assignment ? ExprKind::Assign : ExprKind::Binary;
		node->location = fold->location;
		node->op = fold->op;
		node->alternative = false;
		node->left = left;
		node->right = right_operand;
		return static_cast<Expr*>(node);
	};
	if (right)
	{
		if (accumulated == nullptr)
		{
			accumulated = elements.back();
			elements.pop_back();
		}
		for (size_t i = elements.size(); i-- > 0;)
			accumulated = apply(elements[i], accumulated);
	}
	else
	{
		size_t first = 0;
		if (accumulated == nullptr)
			accumulated = elements[first++];
		for (size_t i = first; i < elements.size(); ++i)
			accumulated = apply(accumulated, elements[i]);
	}
	ParenExpr* paren = static_cast<ParenExpr*>(arena_.allocate(sizeof(ParenExpr), alignof(ParenExpr)));
	*paren = ParenExpr();
	paren->kind = ExprKind::Paren;
	paren->location = fold->location;
	paren->inner = accumulated;
	return paren;
}

Slice<TemplateArgument> SyntaxCloner::expanded_arguments(const Slice<TemplateArgument>& list)
{
	// A template argument list's pack expansions, one argument per element.
	std::vector<TemplateArgument> out;
	for (const TemplateArgument& argument : list)
	{
		TemplateArgument item = argument;
		if (!IsPackArgument(argument))
		{
			item.type = type_id(argument.type);
			item.expression = expr(argument.expression);
			out.push_back(item);
			continue;
		}
		std::vector<const Identifier*> packs;
		const uint32_t location = argument.type != nullptr ? argument.type->location : argument.expression->location;
		const int32_t count = pattern_packs(
			[&]() {
				type_id(argument.type);
				expr(argument.expression);
			},
			packs, location);
		if (count < 0)
		{
			item.type = type_id(argument.type);
			item.expression = expr(argument.expression);
			out.push_back(item);
			continue;
		}
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		saved.swap(renames_);
		item.pack = false;
		for (int32_t i = 0; i < count; ++i)
		{
			rename_to(packs, static_cast<uint32_t>(i));
			item.type = type_id(argument.type);
			// A type-id's pack declarator (`T...`) is the expansion's.
			if (item.type != nullptr && item.type->declarator != nullptr &&
				item.type->declarator->pack != PackPosition::None)
			{
				if (item.type == argument.type)
					item.type = copy(item.type);
				item.type->declarator = copy(item.type->declarator);
				item.type->declarator->pack = PackPosition::None;
			}
			item.expression = expr(argument.expression);
			out.push_back(item);
		}
		renames_.swap(saved);
	}
	if (out.empty())
		return Slice<TemplateArgument>{ nullptr, 0 };
	return Slice<TemplateArgument>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

Slice<TypeId*> SyntaxCloner::expanded_type_ids(const Slice<TypeId*>& list)
{
	// A type-id list whose pack expansions (a type-id's pack declarator,
	// `T...`) become one type-id per element.
	SmallList<TypeId*, 4> out;
	for (TypeId* item : list)
	{
		if (item->declarator == nullptr || item->declarator->pack == PackPosition::None)
		{
			out.push_back(type_id(item));
			continue;
		}
		std::vector<const Identifier*> packs;
		const int32_t count = pattern_packs([&]() { type_id(item); }, packs, item->location);
		if (count < 0)
		{
			out.push_back(type_id(item));
			continue;
		}
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		saved.swap(renames_);
		for (int32_t i = 0; i < count; ++i)
		{
			rename_to(packs, static_cast<uint32_t>(i));
			TypeId* element = type_id(item);
			if (element == item)
				element = copy(element);
			element->declarator = copy(element->declarator);
			element->declarator->pack = PackPosition::None;
			out.push_back(element);
		}
		renames_.swap(saved);
	}
	if (out.empty())
		return Slice<TypeId*>{ nullptr, 0 };
	return Slice<TypeId*>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

ParameterDeclaration* SyntaxCloner::parameter_element(const ParameterDeclaration* parameter)
{
	// One element of a function parameter pack: its declaration without
	// the pack's ellipsis.
	ParameterDeclaration* result = copy(parameter);
	result->specifiers = specifiers(parameter->specifiers);
	Declarator* declarator = this->declarator(parameter->declarator);
	if (declarator == parameter->declarator)
		declarator = copy(declarator);
	declarator->pack = PackPosition::None;
	result->declarator = declarator;
	result->default_argument = initializer(parameter->default_argument);
	return result;
}

Slice<ParameterDeclaration*> SyntaxCloner::expanded_parameters(const Slice<ParameterDeclaration*>& list)
{
	// A function parameter pack (8.3.5p14) becomes one parameter per
	// element of the packs its type names, each named for its element;
	// its own name then denotes a pack of that length in the body.
	std::vector<ParameterDeclaration*> out;
	for (ParameterDeclaration* parameter : list)
	{
		const Declarator* declarator = parameter->declarator;
		if (declarator == nullptr || declarator->pack == PackPosition::None)
		{
			out.push_back(this->parameter(parameter));
			continue;
		}
		std::vector<const Identifier*> packs;
		const int32_t count = pattern_packs(
			[&]() {
				specifiers(parameter->specifiers);
				this->declarator(declarator);
			},
			packs, parameter->location);
		if (count < 0)
		{
			out.push_back(this->parameter(parameter));
			continue;
		}
		const Identifier* id = nullptr;
		for (const Declarator* d = declarator; d != nullptr && id == nullptr; d = d->nested)
		{
			if (d->name != nullptr && d->name->last().kind == NameComponentKind::Identifier)
				id = d->name->last().identifier;
		}
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		saved.swap(renames_);
		for (int32_t i = 0; i < count; ++i)
		{
			rename_to(packs, static_cast<uint32_t>(i));
			if (id != nullptr)
				renames_.push_back(std::make_pair(id, expander_->element_name(id, static_cast<uint32_t>(i))));
			out.push_back(parameter_element(parameter));
		}
		renames_.swap(saved);
		parameter_packs_.push_back(std::make_pair(id, static_cast<uint32_t>(count)));
	}
	if (out.empty())
		return Slice<ParameterDeclaration*>{ nullptr, 0 };
	return Slice<ParameterDeclaration*>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

Slice<BaseSpecifier> SyntaxCloner::bases(const Slice<BaseSpecifier>& list)
{
	// A base-specifier pack expansion (14.5.3p4) names one base per
	// element.
	std::vector<BaseSpecifier> out;
	for (const BaseSpecifier& base : list)
	{
		BaseSpecifier item = base;
		if (!base.pack || expander_ == nullptr)
		{
			item.name = name(base.name);
			out.push_back(item);
			continue;
		}
		std::vector<const Identifier*> packs;
		const int32_t count = pattern_packs([&]() { name(base.name); }, packs, base.location);
		if (count < 0)
		{
			item.name = name(base.name);
			out.push_back(item);
			continue;
		}
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		saved.swap(renames_);
		item.pack = false;
		for (int32_t i = 0; i < count; ++i)
		{
			rename_to(packs, static_cast<uint32_t>(i));
			item.name = name(base.name);
			out.push_back(item);
		}
		renames_.swap(saved);
	}
	if (out.empty())
		return Slice<BaseSpecifier>{ nullptr, 0 };
	return Slice<BaseSpecifier>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

Slice<LambdaCapture> SyntaxCloner::captures(const Slice<LambdaCapture>& list)
{
	// A capture followed by an ellipsis is a pack expansion (5.1.2p23,
	// 14.5.3p4) capturing each element of the function parameter pack it
	// names, as written (by copy or by reference); a capture in a pattern
	// expanded around the lambda names that pattern's element. Shared with
	// the pattern when nothing changes.
	if (expander_ == nullptr)
		return list;
	bool changes = false;
	for (const LambdaCapture& capture : list)
	{
		if (capture.kind == TokenKind::KW_THIS || capture.name == nullptr)
			continue;
		if (collecting_ != nullptr)
		{
			if (!capture.pack && length(capture.name) != -1 &&
				std::find(collecting_->begin(), collecting_->end(), capture.name) == collecting_->end())
				collecting_->push_back(capture.name);
			continue;
		}
		changes = changes || (capture.pack ? length(capture.name) >= 0 : renamed(capture.name) != nullptr);
	}
	if (!changes)
		return list;
	std::vector<LambdaCapture> out;
	for (const LambdaCapture& capture : list)
	{
		const bool named = capture.kind != TokenKind::KW_THIS && capture.name != nullptr;
		const int32_t count = named && capture.pack ? length(capture.name) : -1;
		LambdaCapture item = capture;
		if (count >= 0)
		{
			item.pack = false;
			for (int32_t i = 0; i < count; ++i)
			{
				item.name = expander_->element_name(capture.name, static_cast<uint32_t>(i));
				out.push_back(item);
			}
			continue;
		}
		if (const Identifier* element = named && !capture.pack ? renamed(capture.name) : nullptr)
			item.name = element;
		out.push_back(item);
	}
	if (out.empty())
		return Slice<LambdaCapture>{ nullptr, 0 };
	return Slice<LambdaCapture>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

Slice<MemInitializer> SyntaxCloner::mem_initializers(const Slice<MemInitializer>& list)
{
	// A mem-initializer pack expansion (12.6.2p16) initializes one base
	// per element.
	std::vector<MemInitializer> out;
	for (const MemInitializer& initializer : list)
	{
		MemInitializer item = initializer;
		std::vector<const Identifier*> packs;
		const int32_t count = !initializer.pack || expander_ == nullptr ? -1
																		: pattern_packs(
																			  [&]() {
																				  name(initializer.name);
																				  exprs(initializer.arguments);
																				  expr(initializer.list);
																			  },
																			  packs, initializer.location);
		std::vector<std::pair<const Identifier*, const Identifier*>> saved;
		if (count >= 0)
		{
			saved.swap(renames_);
			item.pack = false;
		}
		for (int32_t i = 0; i < (count < 0 ? 1 : count); ++i)
		{
			if (count >= 0)
				rename_to(packs, static_cast<uint32_t>(i));
			item.name = name(initializer.name);
			item.arguments = exprs(initializer.arguments);
			item.list = static_cast<BracedInitList*>(expr(initializer.list));
			out.push_back(item);
		}
		if (count >= 0)
			renames_.swap(saved);
	}
	if (out.empty())
		return Slice<MemInitializer>{ nullptr, 0 };
	return Slice<MemInitializer>{ arena_.copy_array(out.data(), out.size()), static_cast<uint32_t>(out.size()) };
}

// ------------------------------------------------------ specifiers, types

bool SyntaxCloner::specifier(const DeclSpecifier& in, DeclSpecifier& out)
{
	out = in;
	switch (in.kind)
	{
	case DeclSpecKind::Constinit:
	case DeclSpecKind::Mode:
		return false;
	case DeclSpecKind::Keyword:
		out.operand = expr(in.operand);
		return out.operand != in.operand;
	case DeclSpecKind::Name:
	case DeclSpecKind::Elaborated:
		out.name = name(in.name);
		return out.name != in.name;
	case DeclSpecKind::Decltype:
	case DeclSpecKind::Vector:
		out.operand = expr(in.operand);
		return true;
	case DeclSpecKind::Class:
		out.class_specifier = class_specifier(in.class_specifier);
		return true;
	case DeclSpecKind::Enum:
		out.enum_specifier = enum_specifier(in.enum_specifier);
		return out.enum_specifier != in.enum_specifier;
	}
	return false;
}

Slice<DeclSpecifier> SyntaxCloner::specifiers(const Slice<DeclSpecifier>& list)
{
	DeclSpecifier* out = nullptr;
	for (uint32_t i = 0; i < list.size; ++i)
	{
		DeclSpecifier item;
		const bool changed = specifier(list[i], item);
		if (changed && out == nullptr)
		{
			out = static_cast<DeclSpecifier*>(
				arena_.allocate(sizeof(DeclSpecifier) * list.size, alignof(DeclSpecifier)));
			for (uint32_t j = 0; j < i; ++j)
				out[j] = list[j];
		}
		if (out != nullptr)
			out[i] = item;
	}
	return out == nullptr ? list : Slice<DeclSpecifier>{ out, list.size };
}

Attribute* SyntaxCloner::attribute_list(const Attribute* list)
{
	Attribute* next = attributes(list->next);
	TypeId* type = list->kind == AttributeKind::Mode ? nullptr : type_id(list->type);
	if (next == list->next && (list->kind == AttributeKind::Mode || type == list->type) && list->expression == nullptr)
		return const_cast<Attribute*>(list);
	Attribute* result = copy(list);
	result->next = next;
	if (list->kind != AttributeKind::Mode)
		result->type = type;
	result->expression = expr(list->expression);
	return result;
}

TypeId* SyntaxCloner::type_id(const TypeId* type)
{
	if (type == nullptr)
		return nullptr;
	const Slice<DeclSpecifier> specifiers = this->specifiers(type->specifiers);
	Declarator* declarator = this->declarator(type->declarator);
	if (specifiers.data == type->specifiers.data && declarator == type->declarator)
		return const_cast<TypeId*>(type);
	TypeId* result = copy(type);
	result->specifiers = specifiers;
	result->declarator = declarator;
	return result;
}

bool SyntaxCloner::qualifier(const FunctionQualifier& in, FunctionQualifier& out)
{
	out = in;
	out.noexcept_operand = expr(in.noexcept_operand);
	out.throw_types = pointers(in.throw_types, &SyntaxCloner::type_id);
	out.trailing = type_id(in.trailing);
	return out.noexcept_operand != nullptr || out.throw_types.data != in.throw_types.data ||
		out.trailing != in.trailing;
}

ParameterDeclaration* SyntaxCloner::parameter(const ParameterDeclaration* parameter)
{
	const Slice<DeclSpecifier> specifiers = this->specifiers(parameter->specifiers);
	Declarator* declarator = this->declarator(parameter->declarator);
	Initializer* default_argument = initializer(parameter->default_argument);
	if (specifiers.data == parameter->specifiers.data && declarator == parameter->declarator &&
		default_argument == parameter->default_argument)
		return const_cast<ParameterDeclaration*>(parameter);
	ParameterDeclaration* result = copy(parameter);
	result->specifiers = specifiers;
	result->declarator = declarator;
	result->default_argument = default_argument;
	return result;
}

ParameterClause* SyntaxCloner::parameter_clause(const ParameterClause* clause)
{
	if (clause == nullptr)
		return nullptr;
	if (collecting_ != nullptr)
	{
		// 14.5.3p5: a function parameter pack expands the packs in its
		// declaration, including one inside a type-id of an expression.
		for (const ParameterDeclaration* parameter : clause->parameters)
		{
			const Declarator* declarator = parameter->declarator;
			if (declarator != nullptr && declarator->pack != PackPosition::None)
				nested_expansion([&]() { this->parameter(parameter); });
			else
				this->parameter(parameter);
		}
		return const_cast<ParameterClause*>(clause);
	}
	bool packs = false;
	for (uint32_t i = 0; expander_ != nullptr && i < clause->parameters.size; ++i)
	{
		const Declarator* declarator = clause->parameters[i]->declarator;
		packs = packs || (declarator != nullptr && declarator->pack != PackPosition::None);
	}
	const Slice<ParameterDeclaration*> parameters = packs ? expanded_parameters(clause->parameters)
														  : pointers(clause->parameters, &SyntaxCloner::parameter);
	if (parameters.data == clause->parameters.data)
		return const_cast<ParameterClause*>(clause);
	ParameterClause* result = copy(clause);
	result->parameters = parameters;
	return result;
}

bool SyntaxCloner::suffix(const DeclaratorSuffix& in, DeclaratorSuffix& out)
{
	out = in;
	out.parameters = parameter_clause(in.parameters);
	out.bound = expr(in.bound);
	bool changed = out.parameters != in.parameters || out.bound != nullptr;
	FunctionQualifier* qualifiers = nullptr;
	for (uint32_t i = 0; i < in.qualifiers.size; ++i)
	{
		FunctionQualifier item;
		const bool item_changed = qualifier(in.qualifiers[i], item);
		if (item_changed && qualifiers == nullptr)
		{
			qualifiers = static_cast<FunctionQualifier*>(
				arena_.allocate(sizeof(FunctionQualifier) * in.qualifiers.size, alignof(FunctionQualifier)));
			for (uint32_t j = 0; j < i; ++j)
				qualifiers[j] = in.qualifiers[j];
		}
		if (qualifiers != nullptr)
			qualifiers[i] = item;
	}
	if (qualifiers != nullptr)
	{
		out.qualifiers = Slice<FunctionQualifier>{ qualifiers, in.qualifiers.size };
		changed = true;
	}
	return changed;
}

Declarator* SyntaxCloner::declarator(const Declarator* declarator)
{
	if (declarator == nullptr)
		return nullptr;
	bool changed = false;
	PtrOperator* pointers = nullptr;
	for (uint32_t i = 0; i < declarator->pointers.size; ++i)
	{
		Name* owner = name(declarator->pointers[i].class_name);
		if (owner != declarator->pointers[i].class_name && pointers == nullptr)
			pointers = arena_.copy_array(declarator->pointers.data, declarator->pointers.size);
		if (pointers != nullptr)
			pointers[i].class_name = owner;
	}
	Name* id = name(declarator->name);
	Declarator* nested = this->declarator(declarator->nested);
	DeclaratorSuffix* suffixes = nullptr;
	for (uint32_t i = 0; i < declarator->suffixes.size; ++i)
	{
		DeclaratorSuffix item;
		const bool item_changed = suffix(declarator->suffixes[i], item);
		if (item_changed && suffixes == nullptr)
		{
			suffixes = static_cast<DeclaratorSuffix*>(
				arena_.allocate(sizeof(DeclaratorSuffix) * declarator->suffixes.size, alignof(DeclaratorSuffix)));
			for (uint32_t j = 0; j < i; ++j)
				suffixes[j] = declarator->suffixes[j];
		}
		if (suffixes != nullptr)
			suffixes[i] = item;
	}
	Attribute* attributes = this->attributes(declarator->attributes);
	changed = pointers != nullptr || id != declarator->name || nested != declarator->nested || suffixes != nullptr ||
		attributes != declarator->attributes;
	if (!changed)
		return const_cast<Declarator*>(declarator);
	Declarator* result = copy(declarator);
	if (pointers != nullptr)
		result->pointers = Slice<PtrOperator>{ pointers, declarator->pointers.size };
	result->name = id;
	result->nested = nested;
	if (suffixes != nullptr)
		result->suffixes = Slice<DeclaratorSuffix>{ suffixes, declarator->suffixes.size };
	result->attributes = attributes;
	return result;
}

Initializer* SyntaxCloner::initializer(const Initializer* initializer)
{
	if (initializer == nullptr)
		return nullptr;
	if (initializer->expression == nullptr && initializer->arguments.size == 0)
		return const_cast<Initializer*>(initializer);
	Initializer* result = copy(initializer);
	result->expression = expr(initializer->expression);
	result->arguments = exprs(initializer->arguments);
	return result;
}

// ------------------------------------------------------------ expressions

Expr* SyntaxCloner::type_expr(const Expr* expr)
{
	// Operands that name a type: sizeof, alignof, typeid and casts.
	switch (expr->kind)
	{
	case ExprKind::Cast:
	{
		CastExpr* result = copy(static_cast<const CastExpr*>(expr));
		result->type = type_id(result->type);
		result->operand = this->expr(result->operand);
		return result;
	}
	case ExprKind::Sizeof:
	{
		SizeofExpr* result = copy(static_cast<const SizeofExpr*>(expr));
		result->type = type_id(result->type);
		result->operand = this->expr(result->operand);
		return result;
	}
	case ExprKind::TypeTrait:
	{
		TypeTraitExpr* result = copy(static_cast<const TypeTraitExpr*>(expr));
		result->type = type_id(result->type);
		result->operand = this->expr(result->operand);
		return result;
	}
	case ExprKind::BuiltinTrait:
	{
		BuiltinTraitExpr* result = copy(static_cast<const BuiltinTraitExpr*>(expr));
		result->types = expanded_type_ids(result->types);
		result->operand = this->expr(result->operand);
		return result;
	}
	case ExprKind::New:
	{
		NewExpr* result = copy(static_cast<const NewExpr*>(expr));
		result->placement = exprs(result->placement);
		result->type = type_id(result->type);
		result->initializer = initializer(result->initializer);
		result->constructor = nullptr;
		result->count = nullptr;
		result->deallocation = nullptr;
		return result;
	}
	default:
	{
		FunctionalCastExpr* result = copy(static_cast<const FunctionalCastExpr*>(expr));
		result->aggregate = nullptr;
		result->type = specifiers(result->type);
		result->arguments = exprs(result->arguments);
		result->braced = static_cast<BracedInitList*>(this->expr(result->braced));
		return result;
	}
	}
}

Expr* SyntaxCloner::call_like(const Expr* expr)
{
	switch (expr->kind)
	{
	case ExprKind::Call:
	{
		CallExpr* result = copy(static_cast<const CallExpr*>(expr));
		result->callee = this->expr(result->callee);
		result->arguments = exprs(result->arguments);
		return result;
	}
	case ExprKind::Subscript:
	{
		SubscriptExpr* result = copy(static_cast<const SubscriptExpr*>(expr));
		result->base = this->expr(result->base);
		result->index = this->expr(result->index);
		return result;
	}
	case ExprKind::Member:
	{
		MemberExpr* result = copy(static_cast<const MemberExpr*>(expr));
		result->base = this->expr(result->base);
		result->member = name(result->member, false);
		return result;
	}
	case ExprKind::Lambda:
	{
		// The lambda's function parameter packs are named only in it; a
		// copy that expands them keeps their lengths for its analysis.
		const LambdaExpr* lambda = static_cast<const LambdaExpr*>(expr);
		LambdaExpr* result = copy(lambda);
		result->captures = captures(lambda->captures);
		// A templated lambda's own parameter packs are its call operator's
		// specializations' to expand.
		const size_t shadows = shadows_.size();
		if (lambda->template_parameters != nullptr)
			shadow_template_packs(lambda->template_parameters);
		const size_t packs = parameter_packs_.size();
		result->parameters = parameter_clause(result->parameters);
		if (parameter_packs_.size() > packs)
		{
			typedef std::pair<const Identifier*, uint32_t> Pack;
			const size_t count = parameter_packs_.size() - packs;
			Pack* copied = static_cast<Pack*>(arena_.allocate(sizeof(Pack) * count, alignof(Pack)));
			for (size_t i = 0; i < count; ++i)
				new (&copied[i]) Pack(parameter_packs_[packs + i]);
			result->pattern_parameters = lambda->parameters;
			result->parameter_packs = Slice<const Pack>{ copied, static_cast<uint32_t>(count) };
		}
		result->noexcept_operand = this->expr(result->noexcept_operand);
		result->throw_types = pointers(lambda->throw_types, &SyntaxCloner::type_id);
		result->trailing = type_id(result->trailing);
		result->body = compound(result->body);
		parameter_packs_.resize(packs);
		shadows_.resize(shadows);
		return result;
	}
	default:
	{
		BracedInitList* result = copy(static_cast<const BracedInitList*>(expr));
		result->elements = exprs(result->elements);
		result->union_member = nullptr;
		return result;
	}
	}
}

Expr* SyntaxCloner::expr(const Expr* expr)
{
	if (expr == nullptr)
		return nullptr;
	Expr* result = nullptr;
	switch (expr->kind)
	{
	case ExprKind::Id:
	{
		IdExpr* id = copy(static_cast<const IdExpr*>(expr));
		id->name = name(id->name);
		result = id;
		break;
	}
	case ExprKind::Literal:
		result = copy(static_cast<const LiteralExpr*>(expr));
		break;
	case ExprKind::KeywordLiteral:
		result = copy(static_cast<const KeywordLiteralExpr*>(expr));
		if (collecting_ != nullptr && static_cast<const KeywordLiteralExpr*>(expr)->keyword == TokenKind::KW_THIS)
			++this_uses_;
		break;
	case ExprKind::SizeofPack:
		result = copy(static_cast<const SizeofPackExpr*>(expr));
		if (collecting_ != nullptr)
			++pack_sizes_;
		break;
	case ExprKind::Paren:
	{
		ParenExpr* paren = copy(static_cast<const ParenExpr*>(expr));
		paren->inner = this->expr(paren->inner);
		result = paren;
		break;
	}
	case ExprKind::Binary:
	case ExprKind::Assign:
	{
		BinaryExpr* binary = copy(static_cast<const BinaryExpr*>(expr));
		binary->left = this->expr(binary->left);
		binary->right = this->expr(binary->right);
		result = binary;
		break;
	}
	case ExprKind::Conditional:
	{
		ConditionalExpr* conditional = copy(static_cast<const ConditionalExpr*>(expr));
		conditional->condition = this->expr(conditional->condition);
		conditional->then_expr = this->expr(conditional->then_expr);
		conditional->else_expr = this->expr(conditional->else_expr);
		result = conditional;
		break;
	}
	case ExprKind::Unary:
	case ExprKind::Postfix:
	{
		UnaryExpr* unary = copy(static_cast<const UnaryExpr*>(expr));
		unary->operand = this->expr(unary->operand);
		result = unary;
		break;
	}
	case ExprKind::Delete:
	{
		DeleteExpr* deletion = copy(static_cast<const DeleteExpr*>(expr));
		deletion->operand = this->expr(deletion->operand);
		result = deletion;
		break;
	}
	case ExprKind::PackExpansion:
	{
		// A nested expansion's packs are its own (not the enclosing
		// pattern's being collected).
		PackExpansionExpr* expansion = copy(static_cast<const PackExpansionExpr*>(expr));
		if (collecting_ == nullptr)
			expansion->pattern = this->expr(expansion->pattern);
		else
			nested_expansion([&]() { this->expr(expansion->pattern); });
		result = expansion;
		break;
	}
	case ExprKind::Fold:
	{
		// Expanded where its packs are known; a nested fold's packs are its
		// own, as an expansion's.
		const FoldExpr* fold = static_cast<const FoldExpr*>(expr);
		if (expander_ != nullptr && collecting_ == nullptr)
		{
			if (Expr* expanded = expanded_fold(fold))
			{
				result = expanded;
				break;
			}
		}
		FoldExpr* copied = copy(fold);
		if (collecting_ == nullptr)
		{
			copied->pattern = this->expr(copied->pattern);
			copied->init = this->expr(copied->init);
		}
		else
			nested_expansion([&]() {
				this->expr(fold->pattern);
				this->expr(fold->init);
			});
		result = copied;
		break;
	}
	case ExprKind::Throw:
	{
		ThrowExpr* thrown = copy(static_cast<const ThrowExpr*>(expr));
		thrown->operand = this->expr(thrown->operand);
		result = thrown;
		break;
	}
	case ExprKind::Statement:
	{
		StatementExpr* statement = copy(static_cast<const StatementExpr*>(expr));
		statement->body = compound(statement->body);
		result = statement;
		break;
	}
	case ExprKind::Cast:
	case ExprKind::Sizeof:
	case ExprKind::TypeTrait:
	case ExprKind::BuiltinTrait:
	case ExprKind::New:
	case ExprKind::FunctionalCast:
		result = type_expr(expr);
		break;
	case ExprKind::Call:
	case ExprKind::Subscript:
	case ExprKind::Member:
	case ExprKind::Lambda:
	case ExprKind::BracedInit:
		result = call_like(expr);
		break;
	}
	result->facts = nullptr;
	return result;
}

// ------------------------------------------------------------- statements

CompoundStmt* SyntaxCloner::compound(const CompoundStmt* stmt)
{
	if (stmt == nullptr)
		return nullptr;
	CompoundStmt* result = copy(stmt);
	if (stmt->items.size != 0)
	{
		const size_t shadows = shadows_.size();
		Stmt** items = static_cast<Stmt**>(arena_.allocate(sizeof(Stmt*) * stmt->items.size, alignof(Stmt*)));
		for (uint32_t i = 0; i < stmt->items.size; ++i)
			items[i] = this->stmt(stmt->items[i]);
		result->items = Slice<Stmt*>{ items, stmt->items.size };
		shadows_.resize(shadows);
	}
	return result;
}

bool SyntaxCloner::condition(const Condition& in, Condition& out)
{
	out.expression = expr(in.expression);
	out.declaration = nullptr;
	if (in.declaration != nullptr)
	{
		ConditionDeclaration* declaration = copy(in.declaration);
		declaration->specifiers = specifiers(declaration->specifiers);
		declaration->declarator = declarator(declaration->declarator);
		declaration->initializer = initializer(declaration->initializer);
		declaration->declared = nullptr;
		declaration->conversion = nullptr;
		out.declaration = declaration;
		shadow(in.declaration->declarator, -1);
	}
	return true;
}

Slice<Handler> SyntaxCloner::handlers(const Slice<Handler>& list)
{
	if (list.size == 0)
		return list;
	Handler* out = arena_.copy_array(list.data, list.size);
	for (uint32_t i = 0; i < list.size; ++i)
	{
		const size_t shadows = shadows_.size();
		out[i].exception.specifiers = specifiers(list[i].exception.specifiers);
		out[i].exception.declarator = declarator(list[i].exception.declarator);
		shadow(list[i].exception.declarator, -1);
		out[i].body = compound(list[i].body);
		shadows_.resize(shadows);
	}
	return Slice<Handler>{ out, list.size };
}

void SyntaxCloner::shadow_declared(const Decl* decl)
{
	if (decl == nullptr || decl->kind != DeclKind::Simple)
		return;
	for (const InitDeclarator& declarator : static_cast<const SimpleDecl*>(decl)->declarators)
		shadow(declarator.declarator, -1);
}

Stmt* SyntaxCloner::stmt(const Stmt* stmt)
{
	if (stmt == nullptr)
		return nullptr;
	switch (stmt->kind)
	{
	case StmtKind::Compound:
		return compound(static_cast<const CompoundStmt*>(stmt));
	case StmtKind::Ambiguous:
	{
		// The readings (the first's declarations shadow the block's later
		// names, so it is copied last) and the name the choice turns on,
		// chosen again in the copy's analysis (keeping a template
		// definition's dependent choice).
		AmbiguousStmt* result = copy(static_cast<const AmbiguousStmt*>(stmt));
		for (uint32_t i = result->count; i-- > 0;)
			result->readings[i] = this->stmt(result->readings[i]);
		result->name = name(result->name);
		result->chosen = 0;
		return result;
	}
	case StmtKind::Expression:
	{
		ExpressionStmt* result = copy(static_cast<const ExpressionStmt*>(stmt));
		result->expression = expr(result->expression);
		return result;
	}
	case StmtKind::Declaration:
	{
		// A block-scope declaration hides a pack's name to the block's end.
		DeclarationStmt* result = copy(static_cast<const DeclarationStmt*>(stmt));
		result->declaration = decl(result->declaration);
		shadow_declared(result->declaration);
		return result;
	}
	case StmtKind::If:
	{
		const size_t shadows = shadows_.size();
		IfStmt* result = copy(static_cast<const IfStmt*>(stmt));
		condition(static_cast<const IfStmt*>(stmt)->condition, result->condition);
		result->then_branch = this->stmt(result->then_branch);
		result->else_branch = this->stmt(result->else_branch);
		shadows_.resize(shadows);
		return result;
	}
	case StmtKind::Switch:
	case StmtKind::While:
	{
		const size_t shadows = shadows_.size();
		ConditionalLoopStmt* result = copy(static_cast<const ConditionalLoopStmt*>(stmt));
		condition(static_cast<const ConditionalLoopStmt*>(stmt)->condition, result->condition);
		result->body = this->stmt(result->body);
		shadows_.resize(shadows);
		return result;
	}
	case StmtKind::Do:
	{
		DoStmt* result = copy(static_cast<const DoStmt*>(stmt));
		result->body = this->stmt(result->body);
		result->condition = expr(result->condition);
		return result;
	}
	case StmtKind::For:
	{
		const size_t shadows = shadows_.size();
		ForStmt* result = copy(static_cast<const ForStmt*>(stmt));
		result->init_declaration = decl(result->init_declaration);
		shadow_declared(result->init_declaration);
		result->init_expression = expr(result->init_expression);
		if (result->condition != nullptr)
		{
			Condition* condition = static_cast<Condition*>(arena_.allocate(sizeof(Condition), alignof(Condition)));
			this->condition(*result->condition, *condition);
			result->condition = condition;
		}
		result->iteration = expr(result->iteration);
		result->body = this->stmt(result->body);
		shadows_.resize(shadows);
		return result;
	}
	case StmtKind::RangeFor:
	{
		const size_t shadows = shadows_.size();
		RangeForStmt* result = copy(static_cast<const RangeForStmt*>(stmt));
		result->facts = nullptr;
		result->specifiers = specifiers(result->specifiers);
		result->declarator = declarator(result->declarator);
		result->range = expr(result->range);
		shadow(static_cast<const RangeForStmt*>(stmt)->declarator, -1);
		result->body = this->stmt(result->body);
		shadows_.resize(shadows);
		return result;
	}
	case StmtKind::Case:
	{
		CaseStmt* result = copy(static_cast<const CaseStmt*>(stmt));
		result->value = expr(result->value);
		result->body = this->stmt(result->body);
		result->converted = 0;
		return result;
	}
	case StmtKind::Default:
	case StmtKind::Labeled:
	{
		LabeledStmt* result = copy(static_cast<const LabeledStmt*>(stmt));
		result->body = this->stmt(result->body);
		return result;
	}
	case StmtKind::Return:
	case StmtKind::Throw:
	{
		ValueStmt* result = copy(static_cast<const ValueStmt*>(stmt));
		result->value = expr(result->value);
		return result;
	}
	case StmtKind::Try:
	{
		TryStmt* result = copy(static_cast<const TryStmt*>(stmt));
		result->body = compound(result->body);
		result->handlers = handlers(result->handlers);
		return result;
	}
	case StmtKind::Goto:
		return copy(static_cast<const GotoStmt*>(stmt));
	case StmtKind::Break:
	case StmtKind::Continue:
		return copy(stmt);
	case StmtKind::Asm:
	{
		AsmStmt* result = copy(static_cast<const AsmStmt*>(stmt));
		result->program = nullptr;
		for (Slice<AsmOperand>* operands : { &result->outputs, &result->inputs })
		{
			AsmOperand* copies = operands->size == 0 ? nullptr : arena_.copy_array(operands->data, operands->size);
			for (uint32_t i = 0; i < operands->size; ++i)
				copies[i].expression = expr(copies[i].expression);
			operands->data = copies;
		}
		return result;
	}
	}
	return const_cast<Stmt*>(stmt);
}

// ----------------------------------------------------------- declarations

CtorInitializer* SyntaxCloner::ctor_initializer(const CtorInitializer* list)
{
	if (list == nullptr)
		return nullptr;
	CtorInitializer* result = copy(list);
	if (list->initializers.size != 0)
		result->initializers = mem_initializers(list->initializers);
	return result;
}

FunctionBody* SyntaxCloner::body(const FunctionBody& body)
{
	FunctionBody* result = static_cast<FunctionBody*>(arena_.allocate(sizeof(FunctionBody), alignof(FunctionBody)));
	*result = body;
	const bool saved = bodies_;
	// Local classes' member bodies are part of the copied body.
	bodies_ = true;
	result->ctor_initializer = ctor_initializer(body.ctor_initializer);
	result->body = compound(body.body);
	if (body.try_block != nullptr)
	{
		result->try_block = copy(body.try_block);
		result->try_block->body = result->body;
		result->try_block->handlers = handlers(body.try_block->handlers);
	}
	bodies_ = saved;
	return result;
}

Slice<InitDeclarator> SyntaxCloner::init_declarators(const Slice<InitDeclarator>& list)
{
	if (list.size == 0)
		return list;
	InitDeclarator* out = static_cast<InitDeclarator*>(
		arena_.allocate(sizeof(InitDeclarator) * list.size, alignof(InitDeclarator)));
	for (uint32_t i = 0; i < list.size; ++i)
	{
		new (&out[i]) InitDeclarator();
		out[i].declarator = declarator(list[i].declarator);
		out[i].initializer = initializer(list[i].initializer);
		out[i].first_token = list[i].first_token;
		out[i].end_token = list[i].end_token;
	}
	return Slice<InitDeclarator>{ out, list.size };
}

EnumSpecifier* SyntaxCloner::enum_specifier(const EnumSpecifier* specifier)
{
	if (specifier == nullptr)
		return nullptr;
	EnumSpecifier* result = copy(specifier);
	result->name = name(specifier->name);
	result->underlying = type_id(specifier->underlying);
	result->attributes = attributes(specifier->attributes);
	if (specifier->enumerators.size != 0)
	{
		Enumerator* items = arena_.copy_array(specifier->enumerators.data, specifier->enumerators.size);
		for (uint32_t i = 0; i < specifier->enumerators.size; ++i)
			items[i].value = expr(items[i].value);
		result->enumerators = Slice<Enumerator>{ items, specifier->enumerators.size };
	}
	return result;
}

ClassSpecifier* SyntaxCloner::class_specifier(const ClassSpecifier* specifier)
{
	if (specifier == nullptr)
		return nullptr;
	ClassSpecifier* result = copy(specifier);
	result->name = name(specifier->name);
	if (specifier->bases.size != 0)
		result->bases = bases(specifier->bases);
	result->members = pointers(specifier->members, &SyntaxCloner::decl);
	result->attributes = attributes(specifier->attributes);
	result->storage = nullptr;
	return result;
}

Decl* SyntaxCloner::decl(const Decl* decl)
{
	if (decl == nullptr)
		return nullptr;
	switch (decl->kind)
	{
	case DeclKind::Empty:
	case DeclKind::Asm:
	case DeclKind::Access:
	case DeclKind::NamespaceAlias:
	case DeclKind::UsingDirective:
		return const_cast<Decl*>(decl);
	case DeclKind::UsingDeclaration:
	{
		UsingDecl* result = copy(static_cast<const UsingDecl*>(decl));
		result->target = name(result->target);
		return result;
	}
	case DeclKind::Namespace:
	{
		NamespaceDecl* result = copy(static_cast<const NamespaceDecl*>(decl));
		result->declarations = pointers(result->declarations, &SyntaxCloner::decl);
		return result;
	}
	case DeclKind::Linkage:
	{
		LinkageDecl* result = copy(static_cast<const LinkageDecl*>(decl));
		result->declarations = pointers(result->declarations, &SyntaxCloner::decl);
		return result;
	}
	case DeclKind::Alias:
	{
		AliasDecl* result = copy(static_cast<const AliasDecl*>(decl));
		result->type = type_id(result->type);
		result->attributes = attributes(result->attributes);
		result->declared = nullptr;
		return result;
	}
	case DeclKind::ExplicitInstantiation:
	{
		ExplicitInstantiationDecl* result = copy(static_cast<const ExplicitInstantiationDecl*>(decl));
		result->declaration = this->decl(result->declaration);
		return result;
	}
	case DeclKind::Template:
	{
		// Its parameter packs are its instantiations' to expand.
		const size_t shadows = shadows_.size();
		TemplateDecl* result = copy(static_cast<const TemplateDecl*>(decl));
		shadow_template_packs(result->parameters);
		result->declaration = this->decl(result->declaration);
		shadows_.resize(shadows);
		return result;
	}
	case DeclKind::Class:
	{
		ClassDecl* result = copy(static_cast<const ClassDecl*>(decl));
		specifier(static_cast<const ClassDecl*>(decl)->specifier, result->specifier);
		return result;
	}
	case DeclKind::Enum:
	{
		EnumDecl* result = copy(static_cast<const EnumDecl*>(decl));
		result->enum_specifier = enum_specifier(result->enum_specifier);
		return result;
	}
	case DeclKind::StaticAssert:
	{
		const StaticAssertDecl* assertion = static_cast<const StaticAssertDecl*>(decl);
		if (assertion->independent_condition)
			return const_cast<StaticAssertDecl*>(assertion);
		StaticAssertDecl* result = copy(static_cast<const StaticAssertDecl*>(decl));
		result->condition = expr(result->condition);
		return result;
	}
	// A declaration's function parameter packs are named only in it (its
	// declarators and body): a copy of many members keeps none past its
	// member, so pack lookups do not grow with the members copied.
	case DeclKind::SpecialMember:
	{
		const size_t packs = parameter_packs_.size();
		const SpecialMemberDecl* special = static_cast<const SpecialMemberDecl*>(decl);
		SpecialMemberDecl* result = copy(special);
		result->specifiers = specifiers(special->specifiers);
		result->name = name(special->name);
		result->declarator = declarator(special->declarator);
		result->initializer = initializer(special->initializer);
		if (bodies_ && special->definition)
			result->body = *body(special->body);
		result->attributes = attributes(special->attributes);
		result->declared = nullptr;
		parameter_packs_.resize(packs);
		return result;
	}
	case DeclKind::Function:
	{
		const size_t packs = parameter_packs_.size();
		const FunctionDecl* function = static_cast<const FunctionDecl*>(decl);
		FunctionDecl* result = copy(function);
		result->specifiers = specifiers(function->specifiers);
		result->declarator = declarator(function->declarator);
		if (bodies_)
			result->body = *body(function->body);
		result->attributes = attributes(function->attributes);
		result->declared = nullptr;
		parameter_packs_.resize(packs);
		return result;
	}
	case DeclKind::Simple:
	{
		const size_t packs = parameter_packs_.size();
		const SimpleDecl* simple = static_cast<const SimpleDecl*>(decl);
		SimpleDecl* result = copy(simple);
		result->specifiers = specifiers(simple->specifiers);
		result->declarators = init_declarators(simple->declarators);
		result->attributes = attributes(simple->attributes);
		parameter_packs_.resize(packs);
		return result;
	}
	case DeclKind::BitField:
	{
		const BitFieldDecl* field = static_cast<const BitFieldDecl*>(decl);
		BitFieldDecl* result = copy(field);
		result->specifiers = specifiers(field->specifiers);
		if (field->declarators.size != 0)
		{
			BitFieldDeclarator* items = arena_.copy_array(field->declarators.data, field->declarators.size);
			for (uint32_t i = 0; i < field->declarators.size; ++i)
			{
				items[i].declarator = declarator(items[i].declarator);
				items[i].width = expr(items[i].width);
			}
			result->declarators = Slice<BitFieldDeclarator>{ items, field->declarators.size };
		}
		result->attributes = attributes(field->attributes);
		return result;
	}
	}
	return const_cast<Decl*>(decl);
}

} // namespace syntax
