#include "semantic/analysis/analyzer.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace cppgm
{
namespace semantic
{
namespace
{

class SyntaxPairWorklist
{
public:
	SyntaxPairWorklist(NodeId left, NodeId right) : size_(0) { Push(left, right); }
	bool Empty() const { return size_ == 0 && overflow_.empty(); }
	void Push(NodeId left, NodeId right)
	{
		Pair pair = {left, right};
		if (size_ < kInlinePairs && overflow_.empty()) inline_[size_++] = pair;
		else overflow_.push_back(pair);
	}
	std::pair<NodeId, NodeId> Pop()
	{
		Pair pair;
		if (overflow_.empty()) pair = inline_[--size_];
		else { pair = overflow_.back(); overflow_.pop_back(); }
		return std::make_pair(pair.left, pair.right);
	}
private:
	struct Pair { NodeId left; NodeId right; };
	static const std::size_t kInlinePairs = 16;
	Pair inline_[kInlinePairs];
	std::size_t size_;
	std::vector<Pair> overflow_;
};

std::size_t TemplateParameterOrdinal(
	const std::vector<TemplateParameter>& parameters, NameId name)
{
	if (name == 0) return parameters.size();
	for (std::size_t i = 0; i < parameters.size(); ++i)
		if (parameters[i].name == name) return i;
	return parameters.size();
}

}

std::uint32_t NextComparableTemplateSyntaxEdge(const SyntaxArena& arena,
	std::uint32_t edge, bool ignore_global_qualifier)
{
	while (edge != kNoEdge && ignore_global_qualifier &&
		arena.IsTag(arena.EdgeChild(edge), ::cppgm::syntax::STAG_GLOBAL_QUALIFIER))
		edge = arena.NextEdge(edge);
	return edge;
}

bool EquivalentNormalizedTemplateSyntax(const SyntaxArena& arena,
	NodeId left, NodeId right,
	const std::vector<TemplateParameter>& left_parameters,
	const std::vector<TemplateParameter>& right_parameters,
	NodeId left_global_owner, NodeId right_global_owner,
	Program* program, ScopeId left_scope, ScopeId right_scope,
	const std::vector<NameId>* left_function_parameters,
	const std::vector<NameId>* right_function_parameters)
{
	// Compare retained structure without demanding incomplete dependent types;
	// parameter spelling is normalized to its template-clause ordinal.
	if (left == kNoNode || right == kNoNode) return left == right;
	bool normalize_parameters = left_parameters.size() != right_parameters.size();
	for (std::size_t i = 0; !normalize_parameters && i < left_parameters.size(); ++i)
		normalize_parameters = left_parameters[i].name != right_parameters[i].name;
	SyntaxPairWorklist pending(left, right);
	while (!pending.Empty())
	{
		const std::pair<NodeId, NodeId> pair = pending.Pop();
		const NodeId left_node = pair.first;
		const NodeId right_node = pair.second;
		if (arena.TagId(left_node) != arena.TagId(right_node)) return false;
		const NameId left_name = arena.SemanticPayloadId(left_node);
		const NameId right_name = arena.SemanticPayloadId(right_node);
		bool parameter_name = false;
		if (normalize_parameters)
		{
			const std::size_t left_parameter = TemplateParameterOrdinal(
				left_parameters, left_name);
			const std::size_t right_parameter = TemplateParameterOrdinal(
				right_parameters, right_name);
			parameter_name = left_parameter < left_parameters.size() ||
				right_parameter < right_parameters.size();
			if (parameter_name && left_parameter != right_parameter) return false;
		}

		if (left_function_parameters && right_function_parameters)
		{
			const auto reference_name = [&arena](NodeId node) -> NameId {
				if (arena.IsTag(node, ::cppgm::syntax::STAG_ID_EXPRESSION))
				{
					const std::uint32_t edge = arena.FirstEdge(node);
					if (edge == kNoEdge) return arena.SemanticPayloadId(node);
					if (arena.NextEdge(edge) != kNoEdge) return 0;
					node = arena.EdgeChild(edge);
				}
				if (!arena.IsTag(node, ::cppgm::syntax::STAG_STRUCTURED_TYPE_NAME)) return 0;
				const std::uint32_t edge = arena.FirstEdge(node);
				if (edge == kNoEdge || arena.NextEdge(edge) != kNoEdge) return 0;
				const NodeId component = arena.EdgeChild(edge);
				if (!arena.IsTag(component, "name-component") ||
					arena.FirstEdge(component) != kNoEdge) return 0;
				return arena.SemanticPayloadId(component);
			};
			const auto ordinal = [](const std::vector<NameId>& names, NameId name) {
				return name == 0 ? names.size() : static_cast<std::size_t>(
					std::find(names.begin(), names.end(), name) - names.begin());
			};
			const std::size_t left_ordinal = ordinal(*left_function_parameters,
				reference_name(left_node));
			const std::size_t right_ordinal = ordinal(*right_function_parameters,
				reference_name(right_node));
			if (left_ordinal < left_function_parameters->size() ||
				right_ordinal < right_function_parameters->size())
			{
				if (left_ordinal != right_ordinal) return false;
				continue;
			}
		}

		if (!parameter_name && (left_name != right_name ||
			(left_name == 0 && right_name == 0 &&
			 arena.PayloadId(left_node) != arena.PayloadId(right_node))))
		{
			bool structured_wrapper =
				(arena.IsTag(left_node, ::cppgm::syntax::STAG_DECLTYPE_SPECIFIER) ||
				 arena.IsTag(left_node, ::cppgm::syntax::STAG_DECL_SPECIFIER) ||
				 arena.IsTag(left_node, ::cppgm::syntax::STAG_TRAILING_RETURN_TYPE)) &&
				arena.FirstEdge(left_node) != kNoEdge;
			for (std::uint32_t edge = arena.FirstEdge(left_node);
				edge != kNoEdge; edge = arena.NextEdge(edge))
				if (arena.IsTag(
					arena.EdgeChild(edge), "structured-type-name"))
				{
					structured_wrapper = true;
					break;
				}
			bool equivalent_owner_type = false;
			if (!structured_wrapper && program &&
				left_name != 0 && right_name != 0 &&
				left_scope != kNoScope && right_scope != kNoScope)
			{
				const LookupResult left_lookup = program->LookupName(
					left_scope, left_name, LOOKUP_TYPE);
				const LookupResult right_lookup = program->LookupName(
					right_scope, right_name, LOOKUP_TYPE);
				equivalent_owner_type = left_lookup.type != kNoType &&
					left_lookup.type == right_lookup.type;
			}
			if (!structured_wrapper && !equivalent_owner_type) return false;
		}
		const bool ignore_global_qualifier =
			left_node == left_global_owner && right_node == right_global_owner;
		std::uint32_t left_edge = NextComparableTemplateSyntaxEdge(arena,
			arena.FirstEdge(left_node), ignore_global_qualifier);
		std::uint32_t right_edge = NextComparableTemplateSyntaxEdge(arena,
			arena.FirstEdge(right_node), ignore_global_qualifier);
		while (left_edge != kNoEdge && right_edge != kNoEdge)
		{
			pending.Push(arena.EdgeChild(left_edge), arena.EdgeChild(right_edge));
			left_edge = NextComparableTemplateSyntaxEdge(
				arena, arena.NextEdge(left_edge), ignore_global_qualifier);
			right_edge = NextComparableTemplateSyntaxEdge(
				arena, arena.NextEdge(right_edge), ignore_global_qualifier);
		}
		if (left_edge != right_edge) return false;
	}
	return true;
}

}
}
