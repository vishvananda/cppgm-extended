#include "sema/analyzer.h"

#include "lex/identifier_table.h"

namespace sema {

const Type* Analyzer::alignment_type()
{
	if (!cxx17_ || alignment_type_ != nullptr)
		return alignment_type_;
	const LookupResult space = names_.qualified(graph_.global, std_key_, LookupFilter::Namespace);
	if (!space.found() || space.binding->entity == nullptr || space.binding->entity->kind != EntityKind::Namespace)
		return nullptr;
	const LookupResult found = names_.qualified(static_cast<NamespaceEntity*>(space.binding->entity)->scope,
		special_key("align_val_t"), LookupFilter::Ordinary);
	if (found.found() && !found.ambiguous && found.binding->type != nullptr &&
		found.binding->type->unqualified->kind == TypeKind::Enum)
		alignment_type_ = found.binding->type->unqualified;
	return alignment_type_;
}

void Analyzer::note_allocation_alignment(const FunctionEntity* function)
{
	if (!cxx17_)
		return;
	const Type* alignment = alignment_type();
	if (alignment == nullptr)
		return;
	const Type* type = function->type->canonical;
	for (uint32_t i = 1; i < type->param_count && i < 3; ++i)
		if (type->params[i]->unqualified == alignment)
			function->alignment_parameter = i;
}

FunctionEntity* Analyzer::allocation_function(const syntax::NewExpr& expr, const ClassEntity* entity,
	syntax::Slice<syntax::Expr*>& call_arguments, Scope* scope, bool bound)
{
	const Identifier* key = expr.count != nullptr ? new_array_key_ : operator_key(TokenKind::KW_NEW);
	LookupResult found = LookupResult();
	const ClassEntity* naming = nullptr;
	if (!expr.global && entity != nullptr && entity->scope != nullptr)
	{
		found = names_.qualified(entity->scope, key, LookupFilter::Ordinary);
		naming = found.found() ? entity : nullptr;
	}
	// 10.2p7: members of different bases are an invalid set.
	if (found.ambiguous)
	{
		substitution_error(expr.location, "ambiguous class allocation function name");
		return nullptr;
	}
	std::vector<FunctionEntity*>& set = overload_set_;
	if (!found.found() && bound)
	{
		// One its definition's context declares is viable there (the
		// hosts' check), though the instantiation then selects among those
		// declared by its point too, as GCC and Clang resolve it again.
		const bool lexical = lexical_limit_;
		lexical_limit_ = true;
		const LookupResult declared = global_allocation_functions(key);
		lexical_limit_ = lexical;
		if (!declared.found() || declared.binding->kind != BindingKind::Function)
			error(expr.location, "no allocation function for the new-expression");
		set.clear();
		for (const FunctionList* node = declared.functions; node != nullptr; node = node->next)
			set.push_back(static_cast<FunctionEntity*>(node->binding->entity));
		Candidate declared_candidate = Candidate();
		const Resolution declared_resolution = resolve(call_arguments, set, nullptr, declared_candidate);
		if (declared_resolution == Resolution::NoViable && expr.implicit_alignment != 0)
		{
			std::vector<syntax::Expr*> fallback;
			fallback.push_back(call_arguments.data[0]);
			for (uint32_t i = 0; i + 2 < call_arguments.size; ++i)
				fallback.push_back(call_arguments.data[i + 2]);
			const syntax::Slice<syntax::Expr*> unaligned{ fallback.data(), static_cast<uint32_t>(fallback.size()) };
			if (best_viable(unaligned, expr.location, set, nullptr, true).function == nullptr)
				return nullptr;
		}
		else if (declared_resolution != Resolution::Selected &&
			best_viable(call_arguments, expr.location, set, nullptr, true).function == nullptr)
			return nullptr;
	}
	if (!found.found())
		found = global_allocation_functions(key);
	if (!found.found() || found.binding->kind != BindingKind::Function)
		error(expr.location, "no allocation function for the new-expression");
	set.clear();
	for (const FunctionList* node = found.functions; node != nullptr; node = node->next)
		set.push_back(static_cast<FunctionEntity*>(node->binding->entity));
	Candidate chosen = Candidate();
	Resolution resolution = resolve(call_arguments, set, nullptr, chosen);
	if (resolution == Resolution::NoViable && expr.implicit_alignment != 0)
	{
		for (uint32_t i = 0; i + 2 < call_arguments.size; ++i)
			call_arguments.data[i + 1] = call_arguments.data[i + 2];
		--call_arguments.size;
		expr.implicit_alignment = 0;
		resolution = resolve(call_arguments, set, nullptr, chosen);
	}
	if (resolution != Resolution::Selected)
		chosen = best_viable(call_arguments, expr.location, set, nullptr, true);
	FunctionEntity* function = chosen.function;
	if (function == nullptr)
		return nullptr;
	note_allocation_alignment(function);
	check_not_deleted(function, expr.location);
	check_function_access(function, found.functions, naming, nullptr, scope, expr.location);
	convert_arguments(call_arguments, function->type->canonical, &chosen, 0);
	for (uint32_t i = call_arguments.size;
		function->default_arguments != nullptr && i < function->type->canonical->param_count; ++i)
		instantiate_default_argument(function, i);
	odr_use(function, true);
	if (!function->nothrow)
		++potentially_throwing_;
	return function;
}

const FunctionEntity* Analyzer::usual_deallocation(const FunctionList* functions, const Type* object)
{
	const Type* pointer = types_.pointer(types_.fundamental(FundamentalType::Void));
	const Type* size_type = types_.fundamental(FundamentalType::UnsignedLongInt);
	const Type* alignment = cxx17_ ? alignment_type() : nullptr;
	uint64_t bytes = 0, align = 0;
	const bool over_aligned = alignment != nullptr && object != nullptr && type_size(object, bytes, align, 0) &&
		align > 16;
	// Prefer alignment-aware forms for over-aligned objects, alignment-
	// unaware ones otherwise. Unsized wins within each group; a sized
	// member remains a usual function when its unsized form is absent.
	for (unsigned group = 0; group < (alignment != nullptr ? 2u : 1u); ++group)
	{
		const bool aligned = alignment != nullptr && (group == 0 ? over_aligned : !over_aligned);
		for (unsigned sized = 0; sized < 2; ++sized)
			for (const FunctionList* node = functions; node != nullptr; node = node->next)
			{
				const FunctionEntity* function = static_cast<const FunctionEntity*>(node->binding->entity);
				const Type* type = function->type->canonical;
				if (type->variadic || type->param_count != 1 + sized + (aligned ? 1 : 0) ||
					type->params[0] != pointer ||
					(sized && (type->params[1] != size_type || (!function->member && !aligned))) ||
					(aligned && type->params[1 + sized]->unqualified != alignment))
					continue;
				note_allocation_alignment(function);
				return function;
			}
	}
	return nullptr;
}

} // namespace sema
