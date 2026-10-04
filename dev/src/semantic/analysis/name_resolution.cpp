#include "semantic/analysis/analyzer.h"
#include "support/exceptions.h"

#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace cppgm
{
namespace semantic
{

void Analyzer::ValidateDefaultArgumentContext(NodeId expression, ScopeId scope, TypeId target)
{
	if (expression == kNoNode) return;
	const auto check_object = [this, target](BindingId binding) {
		if (binding == kNoBinding) return;
		const BindingRecord& record = program_->bindings[binding];
		if ((record.kind != BIND_VARIABLE && record.kind != BIND_PARAMETER) ||
			record.storage_class == STORAGE_CLASS_STATIC || record.thread_local_storage) return;
		ScopeId owner = record.owner;
		while (owner != kNoScope && program_->KindOfScope(owner) == SCOPE_BLOCK)
			owner = program_->ParentScope(owner);
		if (owner == kNoScope || program_->KindOfScope(owner) != SCOPE_FUNCTION) return;
		// Preserve the existing constant-value boundary; disputed constant
		// default-argument cases do not establish a new rejection oracle.
		if (record.constant && IsIntegral(record.type, true) && IsConst(record.type) &&
			(program_->types.Get(record.type).cv & CV_VOLATILE) == 0 &&
			!program_->types.IsReference(target)) return;
		ThrowSemanticError("default argument uses an automatic local object");
	};
	std::vector<std::pair<NodeId, bool> > pending(1, std::make_pair(expression, false));
	while (!pending.empty())
	{
		const NodeId node = pending.back().first;
		const bool unevaluated = pending.back().second ||
			arena_->IsTag(node, ::cppgm::syntax::STAG_SIZEOF_EXPRESSION) ||
			arena_->IsTag(node, ::cppgm::syntax::STAG_DECLTYPE_SPECIFIER) ||
			PayloadTokenKind(node) == KW_NOEXCEPT;
		pending.pop_back();
		if (PayloadTokenKind(node) == KW_THIS)
			ThrowSemanticError("this is not permitted in a default argument");
		if (arena_->IsTag(node, ::cppgm::syntax::STAG_LAMBDA_EXPRESSION))
		{
			if (unevaluated) ThrowSemanticError("lambda expression in an unevaluated operand");
			const LambdaCaptureUseTable::Fact& fact = lambda_capture_uses_.FindOrBuild(*arena_, node);
			if (fact.captures_this) ThrowSemanticError("default argument lambda captures this");
			for (std::size_t i = 0; i < fact.name_count; ++i)
				check_object(LookupName(scope, lambda_capture_uses_.NameAt(fact, i), LOOKUP_ORDINARY).ordinary);
			continue;
		}
		if (!unevaluated && arena_->IsTag(node, ::cppgm::syntax::STAG_ID_EXPRESSION))
		{
			// Automatic locals have unqualified names. Do not instantiate
			// qualified template names while checking the declaration context.
			const NamePath name = SyntaxNamePath(node);
			if (!name.global && name.Size() == 1)
				check_object(LookupName(scope, name[0], LOOKUP_ORDINARY).ordinary);
		}
		for (std::uint32_t edge = arena_->FirstEdge(node); edge != kNoEdge; edge = arena_->NextEdge(edge))
			pending.push_back(std::make_pair(arena_->EdgeChild(edge), unevaluated));
	}
}

bool Analyzer::LocalClassObjectUseIsValid(
	BindingId binding, ScopeId scope, bool constant_use) const
{
	if (current_class_context_ == kNoEntity || unevaluated_depth_ != 0 ||
		binding == kNoBinding) return true;
	const BindingRecord& record = program_->bindings[binding];
	if ((record.kind != BIND_VARIABLE && record.kind != BIND_PARAMETER) ||
		record.storage_class == STORAGE_CLASS_STATIC ||
		record.thread_local_storage) return true;
	ScopeId owner = record.owner;
	while (owner != kNoScope && program_->KindOfScope(owner) == SCOPE_BLOCK)
		owner = program_->ParentScope(owner);
	if (owner == kNoScope || program_->KindOfScope(owner) != SCOPE_FUNCTION)
		return true;
	if (constant_use && record.constant && IsIntegral(record.type, true) &&
		IsConst(record.type) &&
		(program_->types.Get(record.type).cv & CV_VOLATILE) == 0) return true;
	for (; scope != kNoScope && scope != record.owner;
		scope = program_->ParentScope(scope))
		if (program_->KindOfScope(scope) == SCOPE_CLASS)
		{
			const EntityId entity = program_->EntityForScope(scope);
			if (entity != kNoEntity && !program_->entities[entity].lambda_closure)
				return false;
		}
	return true;
}

NamePath Analyzer::ParseNamePath(const std::string& spelling,
	NamePathParseFamily family)
{
	if (stats_)
	{
		++stats_->name_path_parse_requests;
		++stats_->name_path_parse_families[family];
	}
	NamePath result;
	std::size_t first = 0;
	result.global = spelling.size() >= 2 && spelling[0] == ':' &&
		spelling[1] == ':';
	if (result.global) first = 2;
	std::size_t conversion_terminal = std::string::npos;
	if (spelling.compare(first, 9, "operator ") == 0)
		conversion_terminal = first;
	else
	{
		const std::size_t separator = spelling.find("::operator ", first);
		if (separator != std::string::npos)
			conversion_terminal = separator + 2;
	}
	while (first < spelling.size())
	{
		std::size_t separator = std::string::npos;
		if (first != conversion_terminal)
		{
			std::size_t angle_depth = 0;
			for (std::size_t scan = first; scan + 1 < spelling.size(); ++scan)
			{
				if (spelling[scan] == '<') ++angle_depth;
				else if (spelling[scan] == '>' && angle_depth != 0) --angle_depth;
				else if (spelling[scan] == ':' && spelling[scan + 1] == ':' &&
					angle_depth == 0)
				{
					separator = scan;
					break;
				}
			}
		}
		const std::size_t last = separator == std::string::npos ?
			spelling.size() : separator;
		if (last == first)
			ThrowSemanticError("invalid qualified name");
		if (first == conversion_terminal)
		{
			std::string terminal;
			terminal.reserve(last - first);
			for (std::size_t i = first; i < last; ++i)
				if (!std::isspace(static_cast<unsigned char>(spelling[i])))
					terminal += spelling[i];
			result.Push(program_->names.Intern(terminal));
		}
		else result.Push(
			program_->names.InternRange(spelling, first, last - first));
		if (stats_) ++stats_->name_path_parse_components;
		if (separator == std::string::npos) break;
		first = separator + 2;
	}
	if (stats_ && result.Size() == 1)
		++stats_->name_path_single_component_parses;
	return result;
}

NamePath Analyzer::GeneratedLibraryPath(GeneratedLibraryName name)
{
	const char* terminal = name == GENERATED_LIBRARY_BAD_ALLOC ? "bad_alloc" :
		name == GENERATED_LIBRARY_TYPE_INFO ? "type_info" : "initializer_list";
	NamePath path;
	path.global = true;
	path.Push(program_->names.Intern("std"));
	path.Push(program_->names.Intern(terminal));
	return path;
}

NamePath Analyzer::StructuredNamePath(NodeId syntax)
{
	if (stats_) ++stats_->structured_name_path_requests;
	NamePath path;
	const NodeId structure = syntax != kNoNode &&
		arena_->IsTag(syntax,
			::cppgm::syntax::STAG_STRUCTURED_TYPE_NAME) ? syntax :
		syntax == kNoNode ? kNoNode :
		FindChild(syntax,
			::cppgm::syntax::STAG_STRUCTURED_TYPE_NAME);
	if (structure == kNoNode) return path;
	path.global = FindChild(structure,
		::cppgm::syntax::STAG_GLOBAL_QUALIFIER) != kNoNode;
	for (std::uint32_t edge = arena_->FirstEdge(structure); edge != kNoEdge;
		edge = arena_->NextEdge(edge))
	{
		const NodeId child = arena_->EdgeChild(edge);
		if (arena_->IsTag(child,
			::cppgm::syntax::STAG_NAME_COMPONENT))
			path.Push(program_->names.UseInterned(
				arena_->SemanticPayloadId(child)));
	}
	return path;
}

NamePath Analyzer::SyntaxNamePath(NodeId syntax)
{
	if (stats_) ++stats_->syntax_name_path_requests;
	NamePath path = StructuredNamePath(syntax);
	if (!path.Empty()) return path;
	if (syntax != kNoNode && arena_->HasSemanticPayload(syntax))
	{
		if (stats_) ++stats_->syntax_name_path_direct;
		path.Push(program_->names.UseInterned(
			arena_->SemanticPayloadId(syntax)));
		return path;
	}
	if (stats_)
	{
		++stats_->syntax_name_path_fallbacks;
		const syntax::SyntaxTagCode tag = syntax == kNoNode ?
			syntax::STAG_NONE : arena_->TagCode(syntax);
		++stats_->syntax_name_path_fallback_tags[tag];
	}
	return syntax == kNoNode ? path : ParseNamePath(
		PayloadSource(syntax), NAME_PATH_PARSE_SYNTAX_FALLBACK);
}

LookupResult Analyzer::LookupSyntaxName(NodeId syntax, ScopeId scope,
	LookupKind kind)
{
	const NodeId structure = syntax == kNoNode ? kNoNode : FindChild(
		syntax, ::cppgm::syntax::STAG_STRUCTURED_TYPE_NAME);
	return structure != kNoNode || (syntax != kNoNode && arena_->IsTag(syntax,
		::cppgm::syntax::STAG_STRUCTURED_TYPE_NAME)) ?
		LookupStructuredName(syntax, scope, kind) :
		LookupPath(scope, SyntaxNamePath(syntax), kind);
}

LookupResult Analyzer::LookupSpelling(ScopeId scope,
	const std::string& spelling, LookupKind kind,
	NamePathParseFamily family)
{
	if (stats_) ++stats_->lookup_spelling_requests;
	return LookupPath(scope, ParseNamePath(spelling, family), kind);
}

ScopeId Analyzer::ResolveScopePath(ScopeId scope,
	const NamePath& path)
{
	const LookupResult result = LookupPath(scope, path, LOOKUP_SCOPE_CARRIER);
	if (result.type != kNoType) EnsureClassDefinition(result.type);
	return result.name_space != kNoScope ? result.name_space :
		result.type != kNoType ? program_->ScopeForType(result.type) : kNoScope;
}

ScopeId Analyzer::ResolveScopeSpelling(ScopeId scope,
	const std::string& spelling, NamePathParseFamily family)
{
	return ResolveScopePath(scope, ParseNamePath(spelling, family));
}

ScopeId Analyzer::ResolveOwner(ScopeId scope, const NamePath& name)
{
	if (!name.global && name.Size() <= 1) return scope;
	NamePath owner = name;
	if (!owner.Empty()) owner.Pop();
	if (owner.Empty()) return owner.global ? program_->GlobalScope() : scope;
	const ScopeId lookup_scope = owner.global ?
		program_->GlobalScope() : scope;
	const LookupResult result =
		LookupPath(lookup_scope, owner, LOOKUP_SCOPE_CARRIER);
	return result.name_space != kNoScope ? result.name_space :
		result.type != kNoType ? program_->ScopeForType(result.type) : kNoScope;
}

}
}
