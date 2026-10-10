#include "sema/semantic_dump.h"

#include <vector>

#include "lex/identifier_table.h"
#include "sema/integral_arithmetic.h"
#include "sema/template_records.h"

namespace sema {

namespace {

const char* ClassKeySpelling(TokenKind key)
{
	switch (key)
	{
	case TokenKind::KW_CLASS:
		return "class";
	case TokenKind::KW_UNION:
		return "union";
	default:
		return "struct";
	}
}

void AppendCv(uint8_t cv, std::string& out)
{
	if (cv & kConst)
		out += "const ";
	if (cv & kVolatile)
		out += "volatile ";
}

const char* EntityLabel(const Entity* entity)
{
	if (entity->kind == EntityKind::Class)
	{
		const ClassEntity* type = static_cast<const ClassEntity*>(entity);
		if (type->label != nullptr)
			return type->label;
	}
	if (entity->kind == EntityKind::Enum)
	{
		const EnumEntity* type = static_cast<const EnumEntity*>(entity);
		if (type->label != nullptr)
			return type->label;
	}
	return entity->name != nullptr ? entity->name->spelling : "";
}

// A declaration's printed name.
const char* BindingName(const Binding* binding)
{
	if (binding->label != nullptr)
		return binding->label;
	if (binding->name != nullptr)
		return binding->name->spelling;
	return binding->entity != nullptr ? EntityLabel(binding->entity) : "";
}

// The class or enumeration spelling a declaration wrote: its key and name.
void AppendDeclaredType(const Binding* declaration, std::string& out)
{
	if (declaration->key == TokenKind::KW_ENUM)
		out += declaration->scoped ? "enum class " : "enum ";
	else
	{
		out += ClassKeySpelling(declaration->key);
		out += ' ';
	}
	out += BindingName(declaration);
}

void AppendValue(const Constant& value, std::string& out)
{
	if (IsSignedConstant(value) && static_cast<int64_t>(value.bits) < 0)
	{
		out += '-';
		out += std::to_string(0 - value.bits);
	}
	else
		out += std::to_string(value.bits);
}

class DumpWriter
{
public:
	explicit DumpWriter(std::ostream& out) : out_(out) {}
	~DumpWriter() { flush(); }

	void scope(const Scope* scope, unsigned depth);

private:
	void indent(unsigned depth) { line_.append(depth * 2, ' '); }
	void header(const Scope* scope, unsigned depth);
	void binding(const Binding* binding, unsigned depth);
	void end_line()
	{
		line_ += '\n';
		if (line_.size() > (1u << 16))
			flush();
	}
	void flush()
	{
		out_.write(line_.data(), static_cast<std::streamsize>(line_.size()));
		line_.clear();
	}

	std::ostream& out_;
	std::string line_;
};

void DumpWriter::header(const Scope* scope, unsigned depth)
{
	indent(depth);
	switch (scope->kind)
	{
	case ScopeKind::Namespace:
		line_ += "scope namespace ";
		break;
	case ScopeKind::TemplateParameters:
		line_ += "scope template-parameters";
		break;
	case ScopeKind::Class:
		line_ += "scope class ";
		break;
	case ScopeKind::Enum:
		line_ += "scope enum ";
		break;
	case ScopeKind::Function:
		line_ += "scope function ";
		break;
	case ScopeKind::Block:
		line_ += "scope block";
		break;
	}
	if (scope->label != nullptr)
		line_ += scope->label;
	end_line();
}

void DumpWriter::binding(const Binding* binding, unsigned depth)
{
	indent(depth);
	switch (binding->kind)
	{
	case BindingKind::Type:
		line_ += "type ";
		line_ += BindingName(binding);
		line_ += ' ';
		if (binding->entity->kind == EntityKind::Class || binding->entity->kind == EntityKind::Enum)
			AppendDeclaredType(binding, line_);
		else
			AppendTypeSpelling(binding->entity->type, line_);
		break;
	case BindingKind::TypeAlias:
		line_ += "type-alias ";
		line_ += BindingName(binding);
		line_ += ' ';
		AppendTypeSpelling(binding->type, line_);
		break;
	case BindingKind::Enumerator:
		line_ += "enumerator ";
		line_ += BindingName(binding);
		line_ += ' ';
		AppendDeclaredType(binding->declaration, line_);
		line_ += ' ';
		AppendValue(static_cast<const EnumeratorEntity*>(binding->entity)->value, line_);
		break;
	case BindingKind::Function:
		line_ += "function ";
		line_ += BindingName(binding);
		line_ += ' ';
		AppendTypeSpelling(binding->type, line_);
		break;
	case BindingKind::Variable:
		// A variable's declarations share its (completed) type.
		line_ += "variable ";
		line_ += BindingName(binding);
		line_ += ' ';
		AppendTypeSpelling(binding->entity->type, line_);
		break;
	case BindingKind::Parameter:
		line_ += "parameter ";
		if (binding->name != nullptr)
			line_ += binding->name->spelling;
		line_ += ' ';
		AppendTypeSpelling(binding->type, line_);
		break;
	default:
		break;
	}
	end_line();
}

void DumpWriter::scope(const Scope* root, unsigned depth)
{
	// Scopes nest as deeply as the source's blocks and classes: an explicit
	// stack of (scope, depth) keeps the walk off the call stack.
	std::vector<std::pair<const Scope*, unsigned>> stack;
	stack.push_back(std::make_pair(root, depth));
	std::vector<const Scope*> children;
	while (!stack.empty())
	{
		const Scope* scope = stack.back().first;
		const unsigned level = stack.back().second;
		stack.pop_back();
		header(scope, level);
		for (const Binding* binding = scope->bindings; binding != nullptr; binding = binding->next)
		{
			if (!binding->hidden)
				this->binding(binding, level + 1);
		}
		children.clear();
		for (const Scope* child = scope->children; child != nullptr; child = child->next_sibling)
		{
			if (child->dumped)
				children.push_back(child);
		}
		for (size_t i = children.size(); i-- > 0;)
			stack.push_back(std::make_pair(children[i], level + 1));
	}
}

} // namespace

void AppendTypeSpelling(const Type* type, std::string& out)
{
	// Pointer, reference and array chains are followed in a loop; function
	// types recurse into their parameters and return type.
	for (;;)
	{
		if (type->kind != TypeKind::Array && type->kind != TypeKind::Function && !type->is_reference())
			AppendCv(type->cv, out);
		switch (type->kind)
		{
		case TypeKind::Fundamental:
			out += FundamentalTypeName(type->fundamental);
			return;
		case TypeKind::Class:
			out += ClassKeySpelling(static_cast<const ClassEntity*>(type->entity)->key);
			out += ' ';
			out += EntityLabel(type->entity);
			return;
		case TypeKind::Enum:
			out += static_cast<const EnumEntity*>(type->entity)->scoped ? "enum class " : "enum ";
			out += EntityLabel(type->entity);
			return;
		case TypeKind::TemplateParameter:
			out += "typename ";
			out += EntityLabel(type->entity);
			return;
		case TypeKind::Template:
			out += "template ";
			out += type->entity->name != nullptr ? type->entity->name->spelling : "";
			return;
		case TypeKind::Value:
			// A non-type template argument: its value; a pointer or
			// reference argument the entity it designates.
			if (AddressArgument(type))
			{
				out += type->entity == nullptr ? "nullptr" : type->element->is_reference() ? "" : "&";
				out += type->entity != nullptr && type->entity->name != nullptr ? type->entity->name->spelling : "";
				return;
			}
			out += IsSignedConstant(Constant{ type->bound, type->element })
				? std::to_string(static_cast<long long>(type->bound))
				: std::to_string(type->bound);
			return;
		case TypeKind::Pack:
			out += "pack of (";
			for (uint32_t i = 0; i < type->param_count; ++i)
			{
				if (i != 0)
					out += ", ";
				AppendTypeSpelling(type->params[i], out);
			}
			out += ')';
			return;
		case TypeKind::Expansion:
			out += "expansion of ";
			break;
		case TypeKind::BoundPack:
			out += "bound ";
			break;
		case TypeKind::TemplateTemplateParameter:
			out += "template-parameter ";
			out += EntityLabel(type->entity);
			return;
		case TypeKind::Pointer:
			out += "pointer to ";
			break;
		case TypeKind::LValueReference:
			out += "lvalue-reference to ";
			break;
		case TypeKind::RValueReference:
			out += "rvalue-reference to ";
			break;
		case TypeKind::MemberPointer:
			out += "member-pointer of ";
			AppendTypeSpelling(type->owner, out);
			out += " to ";
			break;
		case TypeKind::Complex:
			out += "complex ";
			break;
		case TypeKind::BlockPointer:
			out += "block-pointer to ";
			break;
		case TypeKind::Vector:
			out += "vector of ";
			out += std::to_string(type->bound);
			out += ' ';
			break;
		case TypeKind::Array:
			out += "array of ";
			out += std::to_string(type->bound);
			out += ' ';
			break;
		case TypeKind::Function:
			out += "function of (";
			for (uint32_t i = 0; i < type->param_count; ++i)
			{
				if (i != 0)
					out += ", ";
				AppendTypeSpelling(type->params[i], out);
			}
			if (type->variadic)
				out += type->param_count != 0 ? ", ..." : "...";
			out += ')';
			if (type->function_cv & kConst)
				out += " const";
			if (type->function_cv & kVolatile)
				out += " volatile";
			if (type->ref != RefQualifier::None)
				out += type->ref == RefQualifier::LValue ? " &" : " &&";
			out += " returning ";
			break;
		}
		type = type->element;
	}
}

void PrintTypesDump(const SemanticGraph& graph, std::ostream& out)
{
	DumpWriter writer(out);
	out << "translation-unit\n";
	writer.scope(graph.global, 1);
}

} // namespace sema
