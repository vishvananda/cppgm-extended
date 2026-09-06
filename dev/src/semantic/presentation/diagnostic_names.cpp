// How a diagnostic names a type, a template argument and a specialization.
// The reference presentation prints a class template specialization by the
// emission identity the object model gives it, which a reader cannot use;
// these render the source spelling instead, without disturbing the
// presentation contract the reference outputs depend on.
#include "semantic/analysis/analyzer.h"

#include <string>
#include <vector>

namespace cppgm { namespace semantic {
namespace {

// A class template specialization carries the emission identity the object
// model gives it (`__cppgm_class_template_identity_<pattern>_<arguments>_
// <partition>`), and the type renderer prints that verbatim.  A diagnostic
// names the specialization the way the source wrote it instead.
std::string StripTypeIntroducer(const std::string& text)
{
	static const char* const introducers[] =
		{"class ", "struct ", "union ", "enum "};
	for (std::size_t i = 0; i < sizeof(introducers) / sizeof(introducers[0]); ++i)
	{
		const std::string introducer = introducers[i];
		if (text.compare(0, introducer.size(), introducer) == 0)
			return text.substr(introducer.size());
	}
	return text;
}

}  // namespace

// `name<A, B>` for a specialization a diagnostic is about.
std::string Analyzer::DescribeTemplateSpecialization(NameId name,
	const std::vector<TemplateArgument>& arguments) const
{
	std::string rendered = program_->names.Get(name);
	if (arguments.empty()) return rendered;
	rendered += '<';
	for (std::size_t i = 0; i < arguments.size(); ++i)
	{
		if (i != 0) rendered += ", ";
		rendered += DescribeTemplateArgument(arguments[i], 0);
	}
	rendered += '>';
	return rendered;
}

std::string Analyzer::DescribeTemplateArgument(
	const TemplateArgument& argument, std::size_t depth) const
{
	if (argument.kind == TEMPLATE_ARGUMENT_INTEGRAL)
		return std::to_string(argument.value);
	if (argument.type == kNoType) return std::string("...");
	return StripTypeIntroducer(DescribeTypeAt(argument.type, depth + 1));
}

std::string Analyzer::DescribeTypeAt(TypeId type, std::size_t depth) const
{
	std::string text = program_->RenderType(type);
	static const std::string marker = "__cppgm_class_template_identity_";
	if (depth > 4 || text.find(marker) == std::string::npos) return text;
	for (std::size_t entity = 0; entity < program_->entities.size(); ++entity)
	{
		const EntityRecord& record = program_->entities[entity];
		if (record.emission_name == 0 ||
			record.template_argument_begin == kNoBinding) continue;
		const std::string& emission = program_->names.Get(record.emission_name);
		if (emission.compare(0, marker.size(), marker) != 0) continue;
		std::string::size_type at = text.find(emission);
		if (at == std::string::npos) continue;
		if (entity >= class_template_pattern_by_entity_.size()) continue;
		const std::uint32_t pattern = class_template_pattern_by_entity_[entity];
		if (pattern == kNoDumpEdge || pattern >= class_templates_.size()) continue;
		std::string source = program_->names.Get(class_templates_[pattern].name);
		source += '<';
		const std::vector<TemplateArgument> arguments = StoredTemplateArguments(
			record.template_argument_begin, record.template_argument_count);
		for (std::size_t i = 0; i < arguments.size(); ++i)
		{
			if (i != 0) source += ", ";
			source += DescribeTemplateArgument(arguments[i], depth);
		}
		source += '>';
		for (; at != std::string::npos; at = text.find(emission, at + source.size()))
			text.replace(at, emission.size(), source);
	}
	return text;
}

// The type as a diagnostic should name it.
std::string Analyzer::DescribeType(TypeId type) const
{
	return DescribeTypeAt(type, 0);
}

// A specialization named the way the source wrote it, for a diagnostic
// raised while its body is analysed; empty for an ordinary function.
std::string Analyzer::DescribeFunctionSpecialization(BindingId binding) const
{
	if (binding == kNoBinding || binding >= program_->bindings.size())
		return std::string();
	const FunctionInfo& function = GetFunction(binding);
	if (!function.template_specialization ||
		function.template_pattern == kNoDumpEdge ||
		function.template_pattern >= function_templates_.size())
		return std::string();
	// The rendered function type reads "function of (A, B) returning R"; a
	// diagnostic wants the name and the parameters it was called with.
	const std::string rendered = DescribeTypeAt(function.type, 0);
	const std::string introducer = "function of ";
	std::string parameters;
	if (rendered.compare(0, introducer.size(), introducer) == 0)
	{
		const std::string::size_type close = rendered.rfind(") returning ");
		if (close != std::string::npos && close >= introducer.size())
			parameters = rendered.substr(introducer.size(),
				close + 1 - introducer.size());
	}
	return program_->names.Get(
		function_templates_[function.template_pattern].name) + parameters;
}

} }
