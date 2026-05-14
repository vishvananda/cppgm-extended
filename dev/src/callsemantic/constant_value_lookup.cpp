#include "callsemantic/constant_value_lookup.h"

#include "callsemantic/source_location_utils.h"
#include "callsemantic/template_source_utils.h"
#include "callsemantic_internal.h"
#include "constant_value.h"
#include "cpp_decl_ast.h"
#include "cpp_decl_model.h"
#include "parser_trace.h"
#include "semantic_class_model.h"
#include "semantic_context.h"
#include "semantic_context_facets.h"
#include "semantic_errors.h"
#include "semantic_lookup.h"
#include "semantic_trace.h"
#include "semantic_utils.h"
#include "template_argument_semantics.h"
#include "template_api.h"
#include "template_services.h"
#include "types.h"
#include "witness_api.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace callsemantic {
namespace {

using namespace callsemantic_internal;
using namespace cpp_decl;
using namespace semantic_model;
using template_model::TemplateArgument;
using template_model::TemplateParameterInfo;
using semantic_lookup::canonical_function_lookup_name;
using semantic_lookup::MemberValueLookupResult;
using semantic_lookup::lookup_member_value;
using semantic_utils::split_qualified_name_text;
using semantic_utils::strip_trailing_top_level_template_arguments;
using semantic_utils::trim_space;
using semantic_utils::unqualified_member_name;

class ConstantValueLookup
{
public:
  ConstantValueLookup(SemanticContext & ctx_in,
                      const ConstantValueLookupCallbacks & callbacks_in)
    : ctx(ctx_in), callbacks(callbacks_in)
  {}

  operator SemanticContext &() { return ctx; }
  operator const SemanticContext &() const { return ctx; }

  bool materialize_constant_binding_value(ValueBinding & binding,
                                          constant_eval::ConstexprValue & value)
  {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "materialize-binding name=" << binding.name
            << " dependent=" << (binding.dependent_template_value ? "yes" : "no")
            << " has_constant=" << (binding.has_constant_value ? "yes" : "no")
            << " has_constexpr=" << (binding.has_constexpr_value ? "yes" : "no")
            << " has_initializer="
            << (binding.constant_initializer ? "yes" : "no");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(binding.has_constexpr_value) {
      template_api::note_template_member_value_instantiation_if_needed(
          *this,
          binding);
      value = binding.constexpr_value;
      return true;
    }
    if(binding.has_constant_value) {
      template_api::note_template_member_value_instantiation_if_needed(
          *this,
          binding);
      value = constant_eval::make_integral_value(binding.constant_value, binding.type);
      return true;
    }
    if(binding.dependent_template_value ||
       type_depends_on_template_parameter(binding.type)) {
      return false;
    }
    TypePtr binding_base = strip_top_level_cv(remove_reference_type(binding.type));
    if(binding_base &&
       binding_base->kind == Type::TK_ARRAY &&
       !binding.constant_initializer) {
      value = constant_eval::make_array_value(binding.type,
                                              vector<constant_eval::ConstexprValue>(),
                                              binding.name);
      return true;
    }
    if(!binding.constant_initializer ||
       !binding.constant_initializer_scope ||
       binding.constant_value_in_progress) {
      return false;
    }

    binding.constant_value_in_progress = true;
    bool evaluated = false;
    try {
      const std::string initializer_use_location =
          parser_trace::current_use_location().empty() ?
              source_location_for_node(*binding.constant_initializer) :
              std::string();
      const ScopedTemplateUseLocation use_location_guard(
          initializer_use_location);
      evaluated = evaluate_initializer_constant_value(*binding.constant_initializer_scope,
                                                      *binding.constant_initializer,
                                                      binding.type,
                                                      value);
    } catch(...) {
      binding.constant_value_in_progress = false;
      throw;
    }
    binding.constant_value_in_progress = false;
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "materialize-binding-eval name=" << binding.name
            << " evaluated=" << (evaluated ? "yes" : "no");
      if(binding.constant_initializer) {
        trace << " initializer=" << node_text(*binding.constant_initializer);
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!evaluated) {
      return false;
    }

    binding.has_constexpr_value = true;
    binding.constexpr_value = value;
    long long integral = 0;
    if(constant_eval::constexpr_value_to_integral(value, integral)) {
      binding.has_constant_value = true;
      binding.constant_value = integral;
    }
    template_api::note_template_member_value_instantiation_if_needed(
        *this,
        binding);
    return true;
  }

  bool source_template_value_class_use_is_noncanonical(
      Scope & scope,
      const std::vector<std::string> & source_arg_texts,
      const std::vector<TemplateArgument> & resolved_arguments)
  {
    const bool source_mentions_dependent_names =
        template_argument_texts_mention_source_bindings(*this,
                                                       scope,
                                                       source_arg_texts) ||
        template_argument_texts_mention_instantiated_class_local_type_aliases(
            scope,
            source_arg_texts) ||
        template_argument_texts_mention_template_bound_scope_names(
            scope,
            source_arg_texts) ||
        template_argument_texts_mention_enclosing_source_template_parameters(
            scope,
            source_arg_texts);
    if(!source_mentions_dependent_names) {
      return false;
    }
    for(std::size_t i = 0; i < resolved_arguments.size(); ++i) {
      const TemplateArgument & arg = resolved_arguments[i];
      if(arg.kind == TemplateArgument::TA_TYPE &&
         type_depends_on_template_parameter(arg.type)) {
        return true;
      }
      if((arg.kind == TemplateArgument::TA_CLASS_TEMPLATE ||
          arg.kind == TemplateArgument::TA_ALIAS_TEMPLATE) &&
         (arg.dependent || !arg.template_decl)) {
        return true;
      }
    }
    return false;
  }

  bool current_witness_entry_is_closure() const
  {
    return witness::current_template_witness_entry_context().origin ==
           witness::TemplateWitnessOrigin::Closure;
  }

  bool scope_is_inside_instantiated_template_class(Scope & scope) const
  {
    for(Scope * current = &scope; current; current = current->parent) {
      if(current->class_info &&
         current->class_info->source_template &&
         !current->class_info->instantiation_key.empty()) {
        return true;
      }
    }
    return false;
  }

  bool value_class_use_is_from_template_instantiation(Scope & scope) const
  {
    return current_witness_entry_is_closure() ||
           scope_is_inside_instantiated_template_class(scope);
  }

  bool source_argument_texts_differ_from_bindings(
      const std::vector<std::string> & source_arg_texts,
      const std::vector<std::string> & binding_arg_texts) const
  {
    if(source_arg_texts.size() > binding_arg_texts.size()) {
      return true;
    }
    for(std::size_t i = 0; i < source_arg_texts.size(); ++i) {
      if(compact_lookup_text(source_arg_texts[i]) !=
         compact_lookup_text(binding_arg_texts[i])) {
        return true;
      }
    }
    return false;
  }

  bool template_parameter_clause_has_default_before(
      const IRecogTokenSequence & tokens,
      std::size_t open_token,
      std::size_t target_token) const
  {
    bool saw_default = false;
    int angle_depth = 0;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    for(std::size_t i = open_token + 1; i < target_token; ++i) {
      const std::string & source = tokens.peek(i).source;
      if(source == "(") {
        ++paren_depth;
        continue;
      }
      if(source == ")") {
        if(paren_depth > 0) {
          --paren_depth;
        }
        continue;
      }
      if(source == "[") {
        ++bracket_depth;
        continue;
      }
      if(source == "]") {
        if(bracket_depth > 0) {
          --bracket_depth;
        }
        continue;
      }
      if(source == "{") {
        ++brace_depth;
        continue;
      }
      if(source == "}") {
        if(brace_depth > 0) {
          --brace_depth;
        }
        continue;
      }
      if(paren_depth != 0 || bracket_depth != 0 || brace_depth != 0) {
        continue;
      }
      if(source == "<") {
        ++angle_depth;
        continue;
      }
      const int close_count = source == ">" ? 1 : (source == ">>" ? 2 : 0);
      if(close_count != 0) {
        for(int close = 0; close < close_count && angle_depth > 0; ++close) {
          --angle_depth;
        }
        continue;
      }
      if(angle_depth != 0) {
        continue;
      }
      if(source == ",") {
        saw_default = false;
        continue;
      }
      if(source == "=") {
        saw_default = true;
      }
    }
    return saw_default;
  }

  bool source_location_is_template_parameter_default_context_for_witness(
      const std::string & location) const
  {
    const template_api::TemplateWitnessContext witness_context =
        template_witness_context();
    if(!witness::source_capture_enabled(witness_context) ||
       !witness_context.session ||
       !witness_context.source_locations ||
       !witness_context.token_sequence ||
       location.empty()) {
      return false;
    }
    SourceLocationTokenView source_tokens(
        witness_context.source_locations,
        const_cast<IRecogTokenSequence *>(witness_context.token_sequence));
    std::size_t use_token = 0;
    if(!source_tokens.token_index_at_source_location(location, use_token)) {
      return false;
    }

    const IRecogTokenSequence & tokens = *witness_context.token_sequence;
    int angle_depth = 0;
    for(std::size_t i = use_token; i > 0; --i) {
      const std::size_t token_index = i - 1;
      const std::string & source = tokens.peek(token_index).source;
      if(source == ";" || source == "{" || source == "}") {
        return false;
      }
      const int close_count = source == ">" ? 1 : (source == ">>" ? 2 : 0);
      if(close_count != 0) {
        angle_depth += close_count;
        continue;
      }
      if(source != "<") {
        continue;
      }
      if(angle_depth > 0) {
        --angle_depth;
        continue;
      }
      if(token_index > 0 && tokens.peek(token_index - 1).source == "template") {
        return template_parameter_clause_has_default_before(tokens,
                                                            token_index,
                                                            use_token);
      }
    }

    return false;
  }

  bool lookup_constant_template_id_value(
      Scope & scope,
      const TemplateIdSyntax & template_id,
      const string & display_name,
      constant_eval::ConstexprValue & out)
  {
    if(template_id.name.name.empty()) {
      return false;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "lookup-constant template-id=" << display_name
            << " arg_count=" << template_id.arguments.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    VariableTemplateDecl * variable_template =
        lookup_variable_template(scope, template_id.name);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "lookup-constant template-id=" << display_name
            << " variable-template=" << (variable_template ? "found" : "missing");
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!variable_template) {
      return false;
    }

    vector<TemplateArgument> arguments;
    const bool resolved_arguments =
        resolve_template_arguments(scope,
                                   variable_template->parameters,
                                   template_id.arguments,
                                   &template_id.argument_syntaxes,
                                   arguments,
                                   variable_template->declaring_scope);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "lookup-constant template-id=" << display_name
            << " arguments-resolved=" << (resolved_arguments ? "yes" : "no")
            << " count=" << arguments.size();
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!resolved_arguments) {
      return false;
    }

    const auto current_source_template_id_conflicts =
        [&]() -> bool
    {
      const std::string current_location =
          parser_trace::current_use_location();
      if(current_location.empty() ||
         !source_location_points_at_identifier(current_location,
                                               template_id.name.name)) {
        return false;
      }
      const std::vector<std::string> * source_arg_texts =
          template_api::current_template_id_source_arguments_ptr(
              current_location,
              template_id.name.name);
      if(!source_arg_texts) {
        return false;
      }
      if(source_arg_texts->size() > template_id.arguments.size()) {
        return true;
      }
      for(std::size_t i = 0; i < source_arg_texts->size(); ++i) {
        if(compact_lookup_text((*source_arg_texts)[i]) !=
           compact_lookup_text(template_id.arguments[i])) {
          return true;
        }
      }
      return false;
    };
    std::string source_use_location =
        template_api::normalize_template_witness_source_location(
            parser_trace::current_use_location());
    std::string syntax_source_use_location;
    const auto syntax_location = [&]() -> const std::string &
    {
      if(syntax_source_use_location.empty()) {
        syntax_source_use_location =
            template_api::normalize_template_witness_source_location(
                template_api::template_witness_detail::source_location_for_location_id(
                    template_witness_context(), template_id.source_location_id));
      }
      return syntax_source_use_location;
    };
    const bool syntax_source_spells_template =
        source_location_id_points_at_identifier(template_id.source_location_id,
                                                template_id.name.name);
    if(!source_location_points_at_identifier(source_use_location,
                                             template_id.name.name) &&
       syntax_source_spells_template) {
      source_use_location = syntax_location();
    }
    if(source_use_location.empty()) {
      source_use_location = syntax_location();
    }
    const ScopedSuppressedTemplateUseLocation suppress_stale_use_location(
        current_source_template_id_conflicts());
    const ValueBinding * instantiated =
        instantiate_variable_template(scope,
                                      *variable_template,
                                      arguments,
                                      source_use_location);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "lookup-constant template-id=" << display_name
            << " instantiated=" << (instantiated ? "yes" : "no");
      if(instantiated) {
        trace << " dependent="
              << (instantiated->dependent_template_value ? "yes" : "no")
              << " has_constant="
              << (instantiated->has_constant_value ? "yes" : "no");
      }
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return instantiated &&
           materialize_constant_binding_value(const_cast<ValueBinding &>(*instantiated),
                                               out);
  }

  void record_qualified_template_value_class_use(Scope & scope,
                                                 const CppAstNode & node,
                                                 const ValueBinding & binding)
  {
    const bool capture_in_source =
        witness::current_template_witness_entry_context().origin ==
        witness::TemplateWitnessOrigin::Source;
    const bool capture_in_closure =
        witness::current_template_witness_entry_context().origin ==
        witness::TemplateWitnessOrigin::Closure;
    if(!witness::source_capture_enabled(template_witness_context()) ||
       (!capture_in_source && !capture_in_closure) ||
       template_api::function_binding_has_source_template_identity(
           scope.function) ||
       node.qualifier_template_id_syntaxes.empty() ||
       !binding.owner_class ||
       !binding.owner_class->source_template ||
       binding.owner_class->instantiation_arguments.empty()) {
      return;
    }

    const TemplateIdSyntax & qualifier_template_id =
        node.qualifier_template_id_syntaxes.back();
    ClassTemplateDecl * class_template = binding.owner_class->source_template;
    const std::string source_template_name =
        unqualified_member_name(qualifier_template_id.name.name);
    const std::string class_template_name =
        unqualified_member_name(class_template->name);
    if(source_template_name.empty() ||
       class_template_name.empty() ||
       source_template_name != class_template_name) {
      return;
    }

    std::string source_use_location =
        template_api::normalize_template_witness_source_location(
            source_location_for_name_in_node(node, source_template_name, false));
    std::string syntax_source_use_location;
    const auto syntax_location = [&]() -> const std::string &
    {
      if(syntax_source_use_location.empty()) {
        syntax_source_use_location =
            template_api::normalize_template_witness_source_location(
                template_api::template_witness_detail::source_location_for_location_id(
                    template_witness_context(),
                    qualifier_template_id.source_location_id));
      }
      return syntax_source_use_location;
    };
    const bool syntax_source_spells_template =
        source_location_id_points_at_identifier(
            qualifier_template_id.source_location_id,
            source_template_name);
    if(!source_location_points_at_identifier(source_use_location,
                                             source_template_name) &&
       syntax_source_spells_template) {
      source_use_location = syntax_location();
    }
    if(!source_location_points_at_identifier(source_use_location,
                                             source_template_name) ||
       !source_location_identifier_followed_by(source_use_location,
                                              source_template_name,
                                              '<')) {
      return;
    }
    if(source_location_is_template_parameter_default_context_for_witness(
           source_use_location)) {
      return;
    }
    const std::vector<std::string> source_arg_texts =
        template_id_argument_texts_preserving_spacing(qualifier_template_id);

    const std::vector<TemplateArgument> & arguments =
        binding.owner_class->instantiation_arguments;
    const vector<string> binding_arg_texts =
        canonical_instantiation_arg_texts(arguments);
    if(source_template_value_class_use_is_noncanonical(scope,
                                                       source_arg_texts,
                                                       arguments)) {
      return;
    }
    const std::string key =
        template_api::class_template_effective_instantiation_key(
            *this, *binding.owner_class);
    const template_api::specialization::ClassSpecializationSelection selection =
        template_api::specialization::select_class_specialization(
            *this,
            *class_template,
            scope,
            key,
            arguments);

    witness::ClassUseEmitRequest request;
    request.location = source_use_location;
    request.use_anchor_present = true;
    request.use_anchor_location = source_use_location;
    request.template_id_occurrence =
        witness::make_source_template_id_occurrence(
            source_use_location,
            source_arg_texts);
    request.template_id_occurrence.in_template_body =
        value_class_use_is_from_template_instantiation(scope) ||
        source_argument_texts_differ_from_bindings(source_arg_texts,
                                                  binding_arg_texts);
    request.template_name =
        strip_trailing_top_level_template_arguments(
            semantic_model::class_output_qualified_name(*binding.owner_class));
    if(request.template_name.empty()) {
      request.template_name = class_template->name;
    }
    request.selection = source_selection_kind_for_match_kind(selection.kind);
    const witness::TemplateWitnessSourceAnchor selected_decl_anchor =
        class_use_selected_decl_anchor(class_template, selection);
    witness::set_selected_decl_anchor(request.selected_decl_location,
                                      request.selected_decl_anchor,
                                      selected_decl_anchor);
    template_api::append_template_witness_source_bindings(
        *this,
        request.bindings,
        class_template->parameters,
        arguments,
        binding_arg_texts,
        "explicit",
        "defaulted");
    if(selection.parameters && selection.parameters != &class_template->parameters) {
      template_api::append_template_witness_source_bindings(
          *this,
          request.specialization_bindings,
          *selection.parameters,
          selection.arguments,
          "deduced");
    }
    request.role = capture_in_closure ?
        witness::SourceUseRole::QualifierUse :
        witness::SourceUseRole::ValueUse;
    request.ownership = witness::SourceUseOwnership::SourceOwned;
    request.origin = witness::ClassUseEmissionOrigin::QualifiedValueSource;
    witness::emit_class_use(request);
    emit_nested_class_use_source_events_from_location(scope,
                                                      request.location,
                                                      witness::SourceUseOwnership::NestedDerived,
                                                      request.template_name);
  }

  std::string constexpr_call_source_use_location(
      const CppAstNode & callee) const
  {
    std::string location =
        template_api::normalize_template_witness_source_location(
            source_location_for_node(callee));
    if(!location.empty()) {
      return location;
    }
    return template_api::normalize_template_witness_source_location(
        template_api::preferred_fragment_use_location(
            template_witness_context(),
            callee));
  }

  void record_constexpr_direct_call_owner_class_use(
      Scope & scope,
      const CppAstNode & callee,
      FunctionBinding & binding,
      const std::string & use_location)
  {
    if(!witness::source_capture_enabled(template_witness_context()) ||
       witness::current_template_witness_entry_context().origin !=
           witness::TemplateWitnessOrigin::Source ||
       use_location.empty() ||
       !binding.owner_class ||
       !binding.owner_class->source_template ||
       binding.owner_class->instantiation_arguments.empty()) {
      return;
    }

    ClassInfo & owner = *binding.owner_class;
    ClassTemplateDecl * class_template = owner.source_template;
    const std::string source_template_name =
        unqualified_member_name(class_template->name);
    if(!source_template_name.empty() &&
       !source_location_points_at_identifier(use_location,
                                             source_template_name)) {
      return;
    }

    const std::string key =
        template_api::class_template_effective_instantiation_key(*this, owner);
    const template_api::specialization::ClassSpecializationSelection selection =
        template_api::specialization::select_class_specialization(
            *this,
            *class_template,
            scope,
            key,
            owner.instantiation_arguments);

    std::vector<std::string> explicit_arg_texts =
        canonical_instantiation_arg_texts(owner.instantiation_arguments);
    if(!callee.qualifier_template_id_syntaxes.empty()) {
      explicit_arg_texts =
          template_id_argument_texts_preserving_spacing(
              callee.qualifier_template_id_syntaxes.back());
    }

    witness::ClassUseEmitRequest request;
    request.location = use_location;
    request.use_anchor_present = true;
    request.use_anchor_location = use_location;
    request.template_name =
        strip_trailing_top_level_template_arguments(
            semantic_model::class_output_qualified_name(owner));
    if(request.template_name.empty()) {
      request.template_name = class_template->name;
    }
    request.selection = source_selection_kind_for_match_kind(selection.kind);
    witness::set_selected_decl_anchor(
        request.selected_decl_location,
        request.selected_decl_anchor,
        class_use_selected_decl_anchor(class_template, selection));
    template_api::append_template_witness_source_bindings(
        *this,
        request.bindings,
        class_template->parameters,
        owner.instantiation_arguments,
        explicit_arg_texts,
        "explicit",
        "defaulted");
    if(selection.parameters &&
       selection.parameters != &class_template->parameters) {
      template_api::append_template_witness_source_bindings(
          *this,
          request.specialization_bindings,
          *selection.parameters,
          selection.arguments,
          "deduced");
    }
    request.role = witness::SourceUseRole::QualifierUse;
    witness::emit_class_use(request);
  }

  void record_constexpr_direct_function_call_source_use(
      Scope & scope,
      const CppAstNode & callee,
      FunctionBinding & binding,
      const TemplateIdSyntax * template_id_syntax,
      std::size_t explicit_arg_count)
  {
    if(!witness::function_call_source_capture_enabled() ||
       !witness::source_capture_enabled(template_witness_context()) ||
       witness::current_template_witness_entry_context().origin !=
           witness::TemplateWitnessOrigin::Source ||
       witness::template_witness_source_type_lookup_active()) {
      return;
    }

    FunctionTemplateDecl * source_template = binding.source_template;
    if(!source_template && template_id_syntax) {
      const std::string binding_leaf =
          unqualified_member_name(
              canonical_function_lookup_name(binding.name));
      const std::string template_leaf =
          unqualified_member_name(template_id_syntax->name.name);
      std::vector<FunctionTemplateDecl *> templates;
      const std::string lookup_leaf =
          !template_leaf.empty() ? template_leaf : binding_leaf;
      if(binding.owner_class && !lookup_leaf.empty()) {
        const semantic_lookup::MemberFunctionTemplateLookupResult result =
            semantic_lookup::lookup_visible_member_function_templates(
                *binding.owner_class,
                lookup_leaf);
        semantic_lookup::append_unique_function_templates(templates,
                                                          result.templates);
      } else {
        templates = lookup_function_templates(scope,
                                              template_id_syntax->name);
      }
      for(std::size_t i = 0; i < templates.size(); ++i) {
        if(!templates[i]) {
          continue;
        }
        const std::string candidate_leaf =
            unqualified_member_name(
                canonical_function_lookup_name(templates[i]->name));
        if((!template_leaf.empty() && candidate_leaf == template_leaf) ||
           (!binding_leaf.empty() && candidate_leaf == binding_leaf)) {
          source_template = templates[i];
          break;
        }
      }
      if(!source_template && templates.size() == 1) {
        source_template = templates[0];
      }
    }

    const bool function_template_related = source_template != nullptr;
    const bool owner_template_related =
        binding.owner_class != nullptr &&
        binding.owner_class->source_template != nullptr &&
        !binding.owner_class->instantiation_arguments.empty();
    if(!function_template_related && !owner_template_related) {
      return;
    }

    const std::string use_location =
        constexpr_call_source_use_location(callee);
    if(use_location.empty()) {
      return;
    }

    if(owner_template_related) {
      record_constexpr_direct_call_owner_class_use(scope,
                                                   callee,
                                                   binding,
                                                   use_location);
    }
    if(!function_template_related) {
      return;
    }

    witness::FunctionCallSourceDecision decision;
    decision.origin = witness::FunctionCallEmissionOrigin::ConstexprDirectCall;
    witness::set_use_anchor(decision.location,
                            decision.use_anchor,
                            use_location);
    decision.template_name =
        source_template ? source_template->name : binding.name;
    decision.selected =
        semantic_model::function_binding_qualified_name_for_symbol(binding);
    decision.role = witness::SourceUseRole::QualifierUse;
    decision.selection =
        binding.is_explicit_specialization ?
            witness::SourceSelectionKind::ExplicitSpecialization :
            witness::SourceSelectionKind::Instantiation;
    const semantic_model::SourceDeclAnchorCache & decl_anchor =
        source_template ?
            semantic_trace::function_template_decl_anchor(*this, source_template) :
            semantic_trace::function_binding_decl_anchor(*this, &binding);
    witness::set_selected_decl_anchor(decision.selected_decl_location,
                                      decision.selected_decl_anchor,
                                      decl_anchor);
    if(binding.source_template) {
      template_api::append_function_template_witness_bindings(
          *this,
          &binding,
          explicit_arg_count,
          decision.bindings);
    } else if(source_template && template_id_syntax) {
      std::vector<TemplateArgument> function_arguments;
      try
      {
        resolve_template_arguments(scope,
                                   source_template->parameters,
                                   template_id_syntax->arguments,
                                   &template_id_syntax->argument_syntaxes,
                                   function_arguments,
                                   source_template->declaring_scope);
      }
      catch(const TemplateSubstitutionFailure &)
      {
        function_arguments.clear();
      }
      if(!function_arguments.empty() ||
         source_template->parameters.empty()) {
        template_api::append_template_witness_source_bindings(
            *this,
            decision.bindings,
            source_template->parameters,
            function_arguments,
            template_id_argument_texts_preserving_spacing(*template_id_syntax),
            "explicit",
            "defaulted");
      }
    }
    witness::emit_function_call(decision);
  }

  bool lookup_constant_template_member_value(
      Scope & scope,
      const TemplateIdSyntax & qualifier_template_id,
      const string & member_name,
      const string & display_name,
      constant_eval::ConstexprValue & out)
  {
    if(member_name != "value") {
      return false;
    }

    ClassTemplateDecl * class_template =
        lookup_class_template(scope, qualifier_template_id.name);

    const bool std_qualified_is_same =
        !qualifier_template_id.name.qualifiers.empty() &&
        qualifier_template_id.name.qualifiers[0] == "std";
    if(qualifier_template_id.name.name == "is_same" &&
       (!class_template || std_qualified_is_same) &&
       qualifier_template_id.arguments.size() == 2) {
      TypePtr lhs_type;
      TypePtr rhs_type;
      const string lhs_text = trim_space(qualifier_template_id.arguments[0]);
      const string rhs_text = trim_space(qualifier_template_id.arguments[1]);
      if(has_invalid_top_level_qualified_owner_syntax(lhs_text) ||
         has_invalid_top_level_qualified_owner_syntax(rhs_text)) {
        throw logic_error("invalid is_same trait argument type");
      }
      const auto resolve_trait_type_arg =
          [&](size_t index, const string & text, TypePtr & type) -> bool
      {
        type.reset();
        const TemplateArgumentSyntax * syntax =
            index < qualifier_template_id.argument_syntaxes.size() ?
                &qualifier_template_id.argument_syntaxes[index] :
                nullptr;
        if(syntax) {
          if(template_api::type::resolve_type_argument_input(
                 *this, scope, syntax, true, type) &&
             type) {
            return true;
          }
        }
        return false;
      };
      if(!resolve_trait_type_arg(0, lhs_text, lhs_type) ||
         !resolve_trait_type_arg(1, rhs_text, rhs_type) ||
         !lhs_type ||
         !rhs_type ||
         type_depends_on_template_parameter(lhs_type) ||
         type_depends_on_template_parameter(rhs_type)) {
        return false;
      }
      if(parser_trace::enabled("template.resolve")) {
        std::ostringstream trace;
        trace << "lookup-constant template-member=" << display_name
              << " builtin=is_same";
        parser_trace::note("template.resolve", std::string(), trace.str());
      }
      out = constant_eval::make_integral_value(type_equals(lhs_type, rhs_type) ? 1 : 0,
                                               make_fundamental(FT_BOOL));
      return true;
    }

    if(!class_template) {
      return false;
    }

    std::string source_use_location =
        template_api::normalize_template_witness_source_location(
            parser_trace::current_use_location());
    std::string syntax_source_use_location;
    const auto syntax_location = [&]() -> const std::string &
    {
      if(syntax_source_use_location.empty()) {
        syntax_source_use_location =
            template_api::normalize_template_witness_source_location(
                template_api::template_witness_detail::source_location_for_location_id(
                    template_witness_context(),
                    qualifier_template_id.source_location_id));
      }
      return syntax_source_use_location;
    };
    const bool syntax_source_spells_template =
        source_location_id_points_at_identifier(
            qualifier_template_id.source_location_id,
            qualifier_template_id.name.name);
    if(!source_location_points_at_identifier(source_use_location,
                                             qualifier_template_id.name.name) &&
       syntax_source_spells_template) {
      source_use_location = syntax_location();
    }
    if(source_use_location.empty()) {
      source_use_location = syntax_location();
    }
    vector<TemplateArgument> arguments;
    const std::vector<std::string> arg_source_locations =
        class_template_argument_source_locations_for_current_use(
            class_template->name,
            class_template->parameters,
            qualifier_template_id.arguments);
    const template_api::ScopedTemplateArgumentSourceLocations
        template_argument_source_locations(qualifier_template_id.arguments,
                                           arg_source_locations);
    if(!resolve_template_arguments(
           scope,
           class_template->parameters,
           qualifier_template_id.arguments,
           &qualifier_template_id.argument_syntaxes,
           arguments,
           class_template->declaring_scope)) {
      return false;
    }

    const template_api::ClassSpecializationSelection specialization =
        template_api::specialization::select_class_specialization(
            *this,
            *class_template,
            scope,
            template_argument_key(arguments),
            arguments);
    std::string order_use_location =
        template_api::normalize_template_witness_source_location(
            parser_trace::current_order_use_location());
    if(specialization.kind == template_api::MS_EXPLICIT_SPECIALIZATION &&
       specialization.class_node &&
       !order_use_location.empty()) {
      const std::string specialization_location =
          template_api::normalize_template_witness_source_location(
              source_location_for_node(*specialization.class_node));
      if(!specialization_location.empty() &&
         source_location_is_later(order_use_location, specialization_location)) {
        throw ExplicitSpecializationAfterInstantiationError(
            string("explicit specialization after instantiation") +
            " [use " + order_use_location + "]" +
            " [specialization " + specialization_location + "]");
      }
    }
    const parser_trace::ScopedOrderUseLocation order_use_location_guard(
        order_use_location);
    ClassInfo * info =
        instantiate_selected_class_template(
            *class_template,
            scope,
            arguments,
            specialization);
    if(!info) {
      return false;
    }
    if(!order_use_location.empty()) {
      info->first_qualifier_use_location =
          prefer_earlier_source_location(info->first_qualifier_use_location,
                                         order_use_location);
    }
    const std::vector<std::string> source_arg_texts =
        template_id_argument_texts_preserving_spacing(qualifier_template_id);
    const bool source_class_use_is_noncanonical =
        source_template_value_class_use_is_noncanonical(scope,
                                                       source_arg_texts,
                                                       arguments);
    if(witness::source_capture_enabled(template_witness_context()) &&
       !source_use_location.empty() &&
       !source_class_use_is_noncanonical &&
       !source_location_is_template_parameter_default_context_for_witness(
           source_use_location) &&
       source_location_points_at_identifier(source_use_location,
                                            class_template->name)) {
      witness::ClassUseEmitRequest request;
      request.location = source_use_location;
      request.use_anchor_present = true;
      request.use_anchor_location = source_use_location;
      request.template_id_occurrence =
          witness::make_source_template_id_occurrence(
              source_use_location,
              source_arg_texts);
      const vector<string> binding_arg_texts =
          canonical_instantiation_arg_texts(arguments);
      request.template_id_occurrence.in_template_body =
          value_class_use_is_from_template_instantiation(scope) ||
          source_argument_texts_differ_from_bindings(source_arg_texts,
                                                    binding_arg_texts);
      request.template_name =
          !info->qualified_name.empty() ?
              strip_trailing_top_level_template_arguments(info->qualified_name) :
              class_template->name;
      request.selection =
          source_selection_kind_for_match_kind(specialization.kind);
      if(specialization.class_node) {
        request.selected_decl_location =
            template_api::normalize_template_witness_source_location(
                source_location_for_name_in_node(*specialization.class_node,
                                                 class_template->name));
        if(request.selected_decl_location.empty()) {
          request.selected_decl_location =
              template_api::normalize_template_witness_source_location(
                  source_location_for_node(*specialization.class_node));
        }
      }
      witness::set_selected_decl_anchor(request.selected_decl_location,
                                        request.selected_decl_anchor,
                                        request.selected_decl_location,
                                        true);
      template_api::append_template_witness_source_bindings(
          *this,
          request.bindings,
          class_template->parameters,
          arguments,
          source_arg_texts,
          "explicit",
          "defaulted");
      if(specialization.parameters &&
         specialization.parameters != &class_template->parameters) {
        template_api::append_template_witness_source_bindings(
            *this,
            request.specialization_bindings,
            *specialization.parameters,
            specialization.arguments,
            "deduced");
      }
      request.role = value_class_use_is_from_template_instantiation(scope) ?
          witness::SourceUseRole::QualifierUse :
          witness::SourceUseRole::ValueUse;
      request.ownership = witness::SourceUseOwnership::SourceOwned;
      request.origin = witness::ClassUseEmissionOrigin::QualifiedValueSource;
      witness::emit_class_use(request);
    }
    if(member_name == "value") {
      bool structured_bool_value = false;
      if(template_api::with_template_services(
             ctx,
             [&](template_api::TemplateServices & services)
             {
               return template_argument_semantics::
                   structured_bool_constant_value_for_class_info(
                       services,
                       template_api::make_template_environment(scope),
                       *info,
                       structured_bool_value);
             })) {
        MemberValueLookupResult member = lookup_member_value(*info, "value");
        if(member.binding && member.binding->kind != ValueBinding::VK_FIELD) {
          template_api::note_template_member_value_instantiation_if_needed(
              ctx,
              *member.binding);
        }
        template_api::with_template_services(
            ctx,
            [&](template_api::TemplateServices & services)
            {
              template_argument_semantics::
                  note_structured_bool_value_dependencies_for_class_info(
                      services,
                      *info);
            });
        track_standard_invocable_value_chain_for_witness(scope,
                                                         *class_template,
                                                         *info,
                                                         structured_bool_value);
        note_integral_constant_bool_value_for_witness(
            scope,
            class_template->declaring_scope ? *class_template->declaring_scope : scope,
            structured_bool_value);
        out = constant_eval::make_integral_value(
            structured_bool_value ? 1 : 0,
            make_fundamental(FT_BOOL));
        return true;
      }
    }

    finalize_class_constant_members(*info);
    MemberValueLookupResult member = lookup_member_value(*info, member_name);
    return member.binding &&
           materialize_constant_binding_value(const_cast<ValueBinding &>(*member.binding),
                                               out);
  }

  Scope * resolve_qualified_scope_for_node(Scope & scope,
                                           const QualifiedName & qualified,
                                           const CppAstNode & node,
                                           bool allow_dependent_class_qualifiers)
  {
    if(!qualified.rooted && qualified.qualifiers.empty()) {
      return nullptr;
    }

    Scope * current = &scope;
    if(qualified.rooted) {
      while(current->parent) {
        current = current->parent;
      }
    }

    string qualifier_text = qualified.rooted ? string("::") : string();
    Scope * resolved_scope = qualified.rooted ? current : &scope;
    for(size_t i = 0; i < qualified.qualifiers.size(); ++i) {
      if(i != 0) {
        qualifier_text += "::";
      }
      qualifier_text += qualified.qualifiers[i];

      QualifiedName qualifier_name;
      qualifier_name.rooted = qualified.rooted;
      qualifier_name.qualifiers.assign(qualified.qualifiers.begin(),
                                       qualified.qualifiers.begin() + i);
      qualifier_name.name = qualified.qualifiers[i];
      if(Scope * namespace_scope =
             semantic_lookup::lookup_namespace_name(scope, qualifier_name)) {
        current = namespace_scope;
        resolved_scope = namespace_scope;
        continue;
      }

      TypePtr qualifier_type;
      const CppAstNode * qualifier_type_syntax =
          cppast_qualifier_type_syntax(node, i);
      if(qualifier_type_syntax) {
        template_api::type::parse_decltype_or_typeof_node(
            *this, *current, *qualifier_type_syntax, qualifier_type);
      }
      const TemplateIdSyntax * qualifier_template_id =
          cppast_qualifier_template_id_syntax(node, i);
      if(!qualifier_type && qualifier_template_id) {
        TemplateIdSyntax local_qualifier_template_id = *qualifier_template_id;
        if(local_qualifier_template_id.name.rooted ||
           !local_qualifier_template_id.name.qualifiers.empty()) {
          local_qualifier_template_id.name.rooted = false;
          local_qualifier_template_id.name.qualifiers.clear();
          local_qualifier_template_id.name.name =
              unqualified_member_name(local_qualifier_template_id.name.name);
        }
        const string qualifier_source_location =
            template_api::normalize_template_witness_source_location(
                source_location_for_name_in_subtree(
                    node,
                    local_qualifier_template_id.name.name));
        const ScopedTemplateUseLocation qualifier_use_location_guard(
            qualifier_source_location);
        template_api::type::resolve_template_id_syntax_type(
            *this,
            *current,
            local_qualifier_template_id,
            true,
            qualifier_source_location,
            qualifier_type,
            &scope,
            qualifier_source_use_mode(scope,
                                      local_qualifier_template_id));
      }
      if(!qualifier_type && qualifier_template_id) {
        // The structured qualifier failed to resolve; reparsing the same
        // template-id as text hides dependency information.
        return nullptr;
      }
      const bool pause_unspelled_type_source_capture =
          !qualifier_template_id &&
          qualifier_text.find('<') == string::npos;
      const template_api::ScopedTemplateWitnessSourceCapturePause
          source_capture_pause(pause_unspelled_type_source_capture);
      if(!qualifier_type) {
        qualifier_type = lookup_type(*current, qualified.qualifiers[i], true);
        if(!qualifier_type) {
          qualifier_type = lookup_type(scope, qualifier_text, true);
        }
      }
      if(!qualifier_type) {
        return nullptr;
      }
      const template_api::ScopedTemplateWitnessSourceCapturePause
          qualifier_completion_source_capture_pause;
      if(type_depends_on_template_parameter(qualifier_type) &&
         !allow_dependent_class_qualifiers) {
        return nullptr;
      }

      if(Scope * type_scope = ctx.scope_for_type(qualifier_type)) {
        current = type_scope;
        resolved_scope = type_scope;
        continue;
      }

      ClassInfo * qualifier_info = class_info_for_type(qualifier_type);
      if(!qualifier_info || !qualifier_info->member_scope) {
        qualifier_info = complete_class_type(qualifier_type);
      }
      if(!qualifier_info || !qualifier_info->member_scope) {
        return nullptr;
      }
      current = qualifier_info->member_scope.get();
      resolved_scope = current;
    }
    return resolved_scope;
  }

  template_api::ClassTemplateSourceUseMode qualifier_source_use_mode(
      Scope & scope,
      const TemplateIdSyntax & syntax) const
  {
    return (template_api::current_template_witness_entry_context().origin ==
                template_api::TemplateWitnessOrigin::Closure ||
            scope_is_inside_source_template_context(scope) ||
            template_api::function_binding_has_source_template_identity(
                scope.function) ||
            template_id_syntax_has_dependent_source_argument(syntax)) ?
        template_api::ClassTemplateSourceUseMode::NestedArgumentsOnly :
        template_api::ClassTemplateSourceUseMode::EmitClassUse;
  }

  bool template_id_syntax_has_dependent_source_argument(
      const TemplateIdSyntax & syntax) const
  {
    for(size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
      if(syntax.argument_syntaxes[i].dependent) {
        return true;
      }
    }
    return false;
  }

  const ValueBinding * lookup_qualified_value_binding_node(
      Scope & scope,
      const QualifiedName & qualified,
      const CppAstNode & node)
  {
    Scope * target =
        resolve_qualified_scope_for_node(scope, qualified, node, false);
    if(!target) {
      return nullptr;
    }

    map<string, ValueBinding>::const_iterator found =
        target->values.find(qualified.name);
    if(found != target->values.end()) {
      if(target->class_info &&
         !target->class_info->complete &&
         found->second.dependent_template_value &&
         found->second.constant_initializer) {
        if(ClassInfo * completed = complete_class_type(target->class_info->type)) {
          target = completed->member_scope.get();
          found = target->values.find(qualified.name);
          if(found != target->values.end()) {
            return &found->second;
          }
          MemberValueLookupResult member =
              lookup_member_value(*completed, qualified.name);
          if(member.binding) {
            return member.binding;
          }
        }
      }
      return &found->second;
    }
    if(target->class_info) {
      MemberValueLookupResult member =
          lookup_member_value(*target->class_info, qualified.name);
      if(member.binding) {
        return member.binding;
      }
      if(!target->class_info->complete) {
        if(ClassInfo * completed = complete_class_type(target->class_info->type)) {
          target = completed->member_scope.get();
          found = target->values.find(qualified.name);
          if(found != target->values.end()) {
            return &found->second;
          }
          member = lookup_member_value(*completed, qualified.name);
          if(member.binding) {
            return member.binding;
          }
        }
      }
    }
    return nullptr;
  }

  bool lookup_constant_value_node(Scope & scope,
                                  const string & name,
                                  const CppAstNode * node,
                                  constant_eval::ConstexprValue & out)
  {
    if(node &&
       node->kind == CppAstKind::id_expression &&
       (!node->qualifier_type_syntaxes.empty() ||
        !node->qualifier_template_id_syntaxes.empty())) {
      const QualifiedName * qualified = cppast_qualified_name_syntax(*node);
      if(qualified && (qualified->rooted || !qualified->qualifiers.empty())) {
        const ValueBinding * member_binding = nullptr;
        if(lookup_qualified_type_member_constant_value_node(scope,
                                                            *qualified,
                                                            *node,
                                                            out,
                                                            &member_binding)) {
          if(member_binding) {
            record_qualified_template_value_class_use(scope, *node, *member_binding);
          }
          return true;
        }

        const ValueBinding * binding =
            lookup_qualified_value_binding_node(scope, *qualified, *node);
        if(binding &&
           materialize_constant_binding_value(const_cast<ValueBinding &>(*binding), out)) {
          record_qualified_template_value_class_use(scope, *node, *binding);
          return true;
        }
        return false;
      }
    }
    return lookup_constant_value(scope, name, out);
  }

  bool lookup_qualified_type_member_constant_value_node(
      Scope & scope,
      const QualifiedName & qualified,
      const CppAstNode & node,
      constant_eval::ConstexprValue & out,
      const ValueBinding ** member_binding_out = nullptr)
  {
    if(member_binding_out) {
      *member_binding_out = nullptr;
    }

    Scope * target =
        resolve_qualified_scope_for_node(scope, qualified, node, false);
    if(!target || !target->class_info || !target->class_info->type) {
      return false;
    }
    if(!template_api::with_template_services(
           ctx,
           [&](template_api::TemplateServices & services)
           {
             return template_argument_semantics::lookup_type_member_constant_value(
                 services,
                 template_api::make_template_environment(scope),
                 target->class_info->type,
                 qualified.name,
                 out);
           })) {
      return false;
    }

    const ValueBinding * member_binding = nullptr;
    if(member_binding_out) {
      MemberValueLookupResult member =
          lookup_member_value(*target->class_info, qualified.name);
      member_binding = member.binding;
      *member_binding_out = member_binding;
    } else if(qualified.name == "value") {
      MemberValueLookupResult member =
          lookup_member_value(*target->class_info, qualified.name);
      member_binding = member.binding;
    }
    if(qualified.name == "value" && member_binding) {
      require_structured_bool_value_member_output_if_needed(
          const_cast<ValueBinding &>(*member_binding));
    }
    return true;
  }

  void require_structured_bool_value_member_output_if_needed(ValueBinding & binding)
  {
    const template_api::TemplateWitnessContext witness_context =
        template_witness_context();
    if(witness_context.session != nullptr &&
       !witness::source_capture_enabled(witness_context)) {
      return;
    }
    if(binding.kind != ValueBinding::VK_VARIABLE ||
       binding.name != "value" ||
       !binding.type ||
       !is_bool_type(strip_top_level_cv(remove_reference_type(binding.type))) ||
       !binding.owner_class ||
       !binding.owner_class->member_scope) {
      return;
    }

    ClassInfo & owner = *binding.owner_class;
    if(owner.source_template &&
       !owner.out_of_class_static_member_definitions_applied) {
      template_api::TemplateClassFinalizationRequest request;
      if(template_api::build_class_finalization_request(owner, request)) {
        template_api::finalize_class_instantiation(ctx, request);
      }
    }

    map<string, ValueBinding>::iterator required =
        owner.member_scope->values.find(binding.name);
    if(required == owner.member_scope->values.end() ||
       required->second.kind != ValueBinding::VK_VARIABLE ||
       required->second.owner_class != &owner) {
      return;
    }

    add_output_requirement(required->second.output_requirements, ORK_DEFINITION);
    if(parser_trace::enabled("output.require")) {
      std::ostringstream trace;
      trace << "constant-bool-value-output"
            << " class=" << owner.qualified_name
            << " member=" << required->second.name
            << " has-definition=" << (required->second.definition_node ? "yes" : "no")
            << " requirements=" << required->second.output_requirements;
      parser_trace::note("output.require", std::string(), trace.str());
    }
    if(owner.definition_output_emitted &&
       !owner.definition_output_in_progress &&
       !required->second.definition_output_emitted) {
      owner.has_late_required_static_member_output = true;
    }
    ctx.track_instantiated_class(&owner);
  }

  bool lookup_constant_value(Scope & scope,
                             const string & name,
                             constant_eval::ConstexprValue & out)
  {
    const bool witness_capture_enabled =
        witness::source_capture_enabled(template_witness_context());

    const auto qualified_name_text = [](const QualifiedName & qualified_name) -> string
    {
      string out;
      if(qualified_name.rooted) {
        out += "::";
      }
      for(size_t i = 0; i < qualified_name.qualifiers.size(); ++i) {
        out += qualified_name.qualifiers[i];
        out += "::";
      }
      out += qualified_name.name;
      return out;
    };
    const auto qualifier_prefix_text = [](const QualifiedName & qualified_name) -> string
    {
      string out;
      if(qualified_name.rooted) {
        out += "::";
      }
      for(size_t i = 0; i < qualified_name.qualifiers.size(); ++i) {
        if(i != 0) {
          out += "::";
        }
        out += qualified_name.qualifiers[i];
      }
      return out;
    };
    const auto note_single_qualified_class_use =
        [&](const QualifiedName & qualified_name,
            const ValueBinding * resolved_binding) -> void
    {
      if(!witness_capture_enabled ||
         qualified_name.rooted ||
         qualified_name.qualifiers.empty()) {
        return;
      }
      const std::string exact_qualified_use_location =
          earliest_qualified_use_location_for_value(
              qualified_name_text(qualified_name));
      const std::string use_location =
          !exact_qualified_use_location.empty() ?
              exact_qualified_use_location :
              parser_trace::current_use_location();

      if(resolved_binding && resolved_binding->owner_class) {
        std::vector<const ClassInfo *> owner_chain;
        for(const ClassInfo * current = resolved_binding->owner_class;
            current;
            current = current->enclosing_scope ? current->enclosing_scope->class_info : nullptr) {
          owner_chain.push_back(current);
        }
        std::reverse(owner_chain.begin(), owner_chain.end());
        if(owner_chain.size() > 1 && owner_chain.size() <= qualified_name.qualifiers.size()) {
          for(std::size_t i = 0; i < owner_chain.size(); ++i) {
            const ClassInfo * owner = owner_chain[i];
            if(!(owner &&
                 owner->source_template &&
                 !owner->instantiation_arguments.empty())) {
              continue;
            }

            std::string qualifier_prefix;
            for(std::size_t j = 0; j <= i && j < qualified_name.qualifiers.size(); ++j) {
              if(j != 0) {
                qualifier_prefix += "::";
              }
              qualifier_prefix += qualified_name.qualifiers[j];
            }
            std::string event_location =
                earliest_qualified_use_location_for_prefix(qualifier_prefix);
            if(event_location.empty()) {
              event_location = use_location;
            }
            event_location =
                template_api::normalize_template_witness_source_location(event_location);
            if(event_location.empty() ||
               source_location_is_template_parameter_default_context_for_witness(
                   event_location)) {
              continue;
            }

            witness::ClassUseEmitRequest request;
            request.location = event_location;
            request.use_anchor_present = true;
            request.use_anchor_location = event_location;
            request.template_name = strip_trailing_top_level_template_arguments(
                semantic_model::class_output_qualified_name(*owner));
            const std::string source_template_identifier =
                unqualified_member_name(request.template_name);
            if(!source_template_identifier.empty() &&
               !source_location_points_at_identifier(event_location,
                                                     source_template_identifier)) {
              continue;
            }
            request.selection = owner->is_explicit_specialization ?
                witness::SourceSelectionKind::ExplicitSpecialization :
                witness::SourceSelectionKind::Primary;
            const semantic_model::SourceDeclAnchorCache & owner_anchor =
                semantic_trace::class_decl_anchor(*this, owner);
            witness::set_selected_decl_anchor(request.selected_decl_location,
                                              request.selected_decl_anchor,
                                              owner_anchor);
            template_api::append_template_witness_source_bindings(
                *this,
                request.bindings,
                owner->source_template->parameters,
                owner->instantiation_arguments,
                "explicit");
            request.role = witness::SourceUseRole::ValueUse;
            request.origin = witness::ClassUseEmissionOrigin::QualifiedValueSource;
            request.template_id_occurrence.in_template_body =
                value_class_use_is_from_template_instantiation(scope);
            witness::emit_class_use(request);
            emit_nested_class_use_source_events_from_location(scope,
                                                              request.location,
                                                              witness::SourceUseOwnership::NestedDerived,
                                                              request.template_name);
          }
          return;
        }
      }

      if(qualified_name.qualifiers.size() != 1) {
        return;
      }

      ClassTemplateDecl * class_template = nullptr;
      vector<TemplateArgument> arguments;
      vector<string> source_explicit_arg_texts;
      bool has_source_explicit_arg_texts = false;
      string key;
      string qualified_template_name;
      if(resolved_binding && resolved_binding->owner_class) {
        const ClassInfo * owner = resolved_binding->owner_class;
        qualified_template_name = strip_trailing_top_level_template_arguments(
            semantic_model::class_output_qualified_name(*owner));
        class_template = owner->source_template;
        arguments = owner->instantiation_arguments;
        if(class_template && !arguments.empty()) {
          key = template_api::class_template_effective_instantiation_key(*this, *owner);
        }
        if(class_template && qualified_name.qualifiers.size() == 1) {
          const string source_template_name =
              unqualified_member_name(strip_trailing_top_level_template_arguments(
                  qualified_name.qualifiers[0]));
          const string class_template_name =
              unqualified_member_name(class_template->name);
          if(!owner->instantiation_arg_texts.empty() &&
             (qualified_name.qualifiers[0] == class_template->name ||
              (!source_template_name.empty() &&
               source_template_name == class_template->name) ||
              (!source_template_name.empty() &&
               !class_template_name.empty() &&
               source_template_name == class_template_name))) {
            source_explicit_arg_texts = owner->instantiation_arg_texts;
            has_source_explicit_arg_texts = true;
          }
        }
      }

      if(!(class_template && !arguments.empty())) {
        return;
      }
      {
        const string source_template_name =
            unqualified_member_name(strip_trailing_top_level_template_arguments(
                qualified_name.qualifiers[0]));
        const string class_template_name =
            unqualified_member_name(class_template->name);
        if(!source_template_name.empty() &&
           !class_template_name.empty() &&
           source_template_name != class_template_name) {
          return;
        }
      }

      const template_api::specialization::ClassSpecializationSelection selection =
          template_api::specialization::select_class_specialization(
              *this,
              *class_template,
              scope,
              key,
              arguments);

      const witness::TemplateWitnessSourceAnchor selected_decl_anchor =
          class_use_selected_decl_anchor(class_template, selection);
      const std::string selected_decl_location = selected_decl_anchor.location;
      const witness::TemplateWitnessSourceAnchorKind selected_decl_anchor_kind =
          selected_decl_anchor.kind;

      const std::string event_location = !use_location.empty() ?
          use_location :
          selected_decl_location;
      if(event_location.empty() ||
         source_location_is_template_parameter_default_context_for_witness(
             event_location)) {
        return;
      }

      witness::ClassUseEmitRequest request;
      request.location = event_location;
      request.template_name =
          !qualified_template_name.empty() ? qualified_template_name :
                                             class_template->name;
      request.selection = source_selection_kind_for_match_kind(selection.kind);
      if(!exact_qualified_use_location.empty()) {
        request.use_anchor_present = true;
        request.use_anchor_location = exact_qualified_use_location;
      }
      witness::set_selected_decl_anchor(request.selected_decl_location,
                                        request.selected_decl_anchor,
                                        selected_decl_location,
                                        selected_decl_anchor_kind);
      const auto canonicalize_binding_argument_texts =
          [&](std::vector<witness::TemplateWitnessSourceBinding> & bindings,
              const std::vector<TemplateParameterInfo> & parameters_to_use,
              const std::vector<TemplateArgument> & arguments_to_use) -> void
      {
        std::size_t binding_index = 0;
        std::size_t arg_index = 0;
        for(std::size_t i = 0;
            i < parameters_to_use.size() &&
            binding_index < bindings.size();
            ++i) {
          if(parameters_to_use[i].parameter_pack) {
            std::size_t trailing_non_pack = 0;
            for(std::size_t j = i + 1; j < parameters_to_use.size(); ++j) {
              if(!parameters_to_use[j].parameter_pack) {
                ++trailing_non_pack;
              }
            }
            if(arguments_to_use.size() < arg_index + trailing_non_pack) {
              return;
            }
            const std::size_t pack_end = arguments_to_use.size() - trailing_non_pack;
            bindings[binding_index].pack_binding = true;
            bindings[binding_index].pack_aggregate = pack_end > arg_index + 1;
            bindings[binding_index].pack_arguments.clear();
            std::ostringstream pack_text;
            pack_text << "<";
            for(std::size_t j = arg_index; j < pack_end; ++j) {
              if(j != arg_index) {
                pack_text << ", ";
              }
              const std::string element_text =
                  template_argument_text(arguments_to_use[j]);
              bindings[binding_index].pack_arguments.push_back(element_text);
              pack_text << element_text;
            }
            pack_text << ">";
            bindings[binding_index].arg = pack_text.str();
            ++binding_index;
            arg_index = pack_end;
            continue;
          }
          if(arg_index >= arguments_to_use.size()) {
            return;
          }
          bindings[binding_index].pack_binding = false;
          bindings[binding_index].pack_aggregate = false;
          bindings[binding_index].pack_arguments.clear();
          bindings[binding_index].arg =
              template_argument_text(arguments_to_use[arg_index]);
          ++binding_index;
          ++arg_index;
        }
      };
      if(has_source_explicit_arg_texts) {
        template_api::append_template_witness_source_bindings(
            *this,
            request.bindings,
            class_template->parameters,
            arguments,
            source_explicit_arg_texts,
            "explicit",
            "defaulted");
        canonicalize_binding_argument_texts(request.bindings,
                                            class_template->parameters,
                                            arguments);
      } else {
        template_api::append_template_witness_source_bindings(
            *this,
            request.bindings,
            class_template->parameters,
            arguments,
            "explicit");
      }
      if(selection.parameters && selection.parameters != &class_template->parameters) {
        template_api::append_template_witness_source_bindings(
            *this,
            request.specialization_bindings,
            *selection.parameters,
            selection.arguments,
            "deduced");
      }
      request.role = witness::SourceUseRole::ValueUse;
      request.origin = witness::ClassUseEmissionOrigin::QualifiedValueSource;
      request.template_id_occurrence.in_template_body =
          value_class_use_is_from_template_instantiation(scope) ||
          (has_source_explicit_arg_texts &&
           source_argument_texts_differ_from_bindings(
               source_explicit_arg_texts,
               canonical_instantiation_arg_texts(arguments)));
      witness::emit_class_use(request);
      emit_nested_class_use_source_events_from_location(scope,
                                                        request.location,
                                                        witness::SourceUseOwnership::NestedDerived,
                                                        request.template_name);
    };

    QualifiedName qualified;
    if(split_qualified_name_text(name, qualified) &&
       (qualified.rooted || !qualified.qualifiers.empty())) {
      const string qualifier_name = qualifier_prefix_text(qualified);

      const ValueBinding * binding =
          semantic_lookup::lookup_qualified_value_binding(*this, scope, qualified);
      if(binding &&
         materialize_constant_binding_value(const_cast<ValueBinding &>(*binding), out)) {
        note_single_qualified_class_use(qualified, binding);
        return true;
      }

      if(!qualifier_name.empty()) {
        TypePtr qualifier_type = lookup_type(scope, qualifier_name, false);
        ClassInfo * qualifier_info = complete_class_type(qualifier_type);
        if(qualifier_info) {
          finalize_class_constant_members(*qualifier_info);
          MemberValueLookupResult member =
              lookup_member_value(*qualifier_info, qualified.name);
          if(member.binding &&
             materialize_constant_binding_value(const_cast<ValueBinding &>(*member.binding), out)) {
            note_single_qualified_class_use(qualified, member.binding);
            return true;
          }
        }
      }
    }

    const ValueBinding * binding = lookup_value(scope, name);
    if(binding &&
       materialize_constant_binding_value(const_cast<ValueBinding &>(*binding), out)) {
      return true;
    }

    return false;
  }

  bool lookup_constant_value(Scope & scope, const string & name, long long & out)
  {
    constant_eval::ConstexprValue value;
    return lookup_constant_value(scope, name, value) &&
           constant_eval::constexpr_value_to_integral(value, out);
  }


private:
  SemanticContext & ctx;
  const ConstantValueLookupCallbacks & callbacks;


  void emit_nested_class_use_source_events_from_location(
      Scope & scope,
      const std::string & location,
      witness::SourceUseOwnership ownership,
      const std::string & skip_exact_template_name = std::string())
  {
    callbacks.emit_nested_class_use_source_events_from_location(
        scope, location, ownership, skip_exact_template_name);
  }

  std::string template_argument_text(const TemplateArgument & argument) const
  {
    return template_model::template_argument_text(
        argument,
        [this](const TypePtr & type)
        {
          return template_api::type::lookup_text_for_type_argument(ctx, type);
        });
  }

  template_api::TemplateWitnessContext template_witness_context() const
  {
    return ctx.template_witness_context();
  }

  std::string source_location_for_node(const CppAstNode & node) const
  {
    return ctx.source_location_for_node(node);
  }

  std::string source_location_for_name_in_node(const CppAstNode & node,
                                               const std::string & name,
                                               bool prefer_last = false) const
  {
    return ctx.source_location_for_name_in_node(node, name, prefer_last);
  }

  std::string source_location_for_name_in_subtree(const CppAstNode & node,
                                                  const std::string & name,
                                                  bool prefer_last = false) const
  {
    return callbacks.source_location_for_name_in_subtree(node, name, prefer_last);
  }

  bool source_location_points_at_identifier(const std::string & location,
                                            const std::string & identifier) const
  {
    return template_api::template_witness_detail::
        source_location_points_at_identifier_token(ctx.template_witness_context(),
                                                   location,
                                                   identifier);
  }

  bool source_location_id_points_at_identifier(uint32_t location_id,
                                               const std::string & identifier) const
  {
    return template_api::template_witness_detail::
        source_location_id_points_at_identifier_token(
            ctx.template_witness_context(),
            location_id,
            identifier);
  }

  bool source_location_identifier_followed_by(const std::string & location,
                                              const std::string & identifier,
                                              char ch) const
  {
    return template_api::template_witness_detail::
        source_location_identifier_token_followed_by(ctx.template_witness_context(),
                                                     location,
                                                     identifier,
                                                     std::string(1, ch));
  }

  bool source_location_id_identifier_followed_by(uint32_t location_id,
                                                 const std::string & identifier,
                                                 char ch) const
  {
    return template_api::template_witness_detail::
        source_location_id_identifier_token_followed_by(
            ctx.template_witness_context(),
            location_id,
            identifier,
            std::string(1, ch));
  }

  std::string earliest_qualified_use_location_for_prefix(const std::string & prefix) const
  {
    return callbacks.earliest_qualified_use_location_for_prefix(prefix);
  }

  std::string earliest_qualified_use_location_for_value(const std::string & value) const
  {
    return callbacks.earliest_qualified_use_location_for_value(value);
  }

  std::string template_argument_key(const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::template_argument_identity_key(ctx, arguments);
  }

  std::vector<std::string> canonical_instantiation_arg_texts(
      const std::vector<TemplateArgument> & arguments) const
  {
    return template_api::canonical_template_argument_texts(ctx, arguments);
  }

  std::string strip_at_prefix(const std::string & location) const
  {
    if(location.compare(0, 4, " at ") == 0) {
      return location.substr(4);
    }
    return location;
  }

  bool scope_is_std_namespace_or_inline_child(const Scope * scope) const
  {
    const Scope * current = scope;
    while(current && current->namespace_scope && current->inline_namespace) {
      current = current->parent;
    }
    if(!current ||
       !current->namespace_scope ||
       current->name != "std") {
      return false;
    }
    for(const Scope * parent = current->parent; parent; parent = parent->parent) {
      if(parent->namespace_scope &&
         parent->name != "<global>" &&
         parent->name != "<unnamed>") {
        return false;
      }
    }
    return true;
  }

  VariableTemplateDecl * lookup_direct_or_inline_variable_template(Scope & scope,
                                                                   const string & name)
  {
    if(VariableTemplateDecl * direct =
           semantic_lookup::lookup_direct_variable_template(scope, name)) {
      return direct;
    }
    for(size_t i = 0; i < scope.namespace_children.size(); ++i) {
      Scope & child = *scope.namespace_children[i];
      if(!child.inline_namespace && child.name != "<unnamed>") {
        continue;
      }
      if(VariableTemplateDecl * nested =
             lookup_direct_or_inline_variable_template(child, name)) {
        return nested;
      }
    }
    return nullptr;
  }

  void note_variable_template_instantiation_for_witness(
      VariableTemplateDecl & variable_template)
  {
    const std::string entity =
        variable_template.declaring_scope ?
            semantic_lookup::scope_qualified_name(*variable_template.declaring_scope,
                                                  variable_template.name) :
            variable_template.name;
    const std::string decl_location =
        strip_at_prefix(semantic_model::source_decl_anchor_location(
            semantic_trace::variable_template_decl_anchor(ctx, &variable_template)));
    if(entity.empty() || decl_location.empty()) {
      return;
    }
    const witness::ScopedTemplateWitnessEntryContext entry_context(
        witness::make_template_closure_entry_context(
            witness::TemplateClosureReason::TrackInstantiation,
            entity,
            decl_location,
            true));
    witness::note_template_witness_log_event(
        witness::TemplateWitnessLogEventKind::VariableInstantiation,
        decl_location,
        entity,
        decl_location,
        std::string(),
        witness::TemplateLifecycleCause::TrackInstantiation,
        true);
  }

  void track_variable_template_instantiation_for_witness(
      Scope & use_scope,
      Scope & lookup_scope,
      const string & name,
      const vector<TemplateArgument> & arguments)
  {
    if(ctx.template_witness_context().session == nullptr) {
      return;
    }
    VariableTemplateDecl * variable_template =
        lookup_direct_or_inline_variable_template(lookup_scope, name);
    if(!variable_template) {
      return;
    }
    note_variable_template_instantiation_for_witness(*variable_template);
    template_api::TemplateVariableInstantiationRequest request;
    request.decl = variable_template;
    request.arguments = arguments;
    request.source_use_scope = &use_scope;
    request.intent = template_api::TemplateInstantiationIntent::TrackInstantiation;
    const witness::ScopedTemplateWitnessSourceCapturePause pause;
    (void)template_api::acquire_variable_instantiation(ctx, request);
  }

  void note_integral_constant_bool_value_for_witness(Scope & use_scope,
                                                     Scope & lookup_scope,
                                                     bool value)
  {
    if(ctx.template_witness_context().session == nullptr) {
      return;
    }
    ClassTemplateDecl * integral_constant =
        lookup_class_template(lookup_scope, "integral_constant");
    if(!integral_constant) {
      return;
    }
    {
      const std::string decl_location =
          strip_at_prefix(semantic_model::source_decl_anchor_location(
              semantic_trace::class_template_decl_anchor(ctx, integral_constant)));
      std::string entity;
      if(integral_constant->declaring_scope &&
         scope_is_std_namespace_or_inline_child(integral_constant->declaring_scope)) {
        entity = std::string("std::integral_constant<bool, ") +
            (value ? "true" : "false") + ">::value";
      } else {
        entity = std::string("integral_constant<bool, ") +
            (value ? "true" : "false") + ">::value";
      }
      if(!decl_location.empty()) {
        const witness::ScopedTemplateWitnessEntryContext entry_context(
            witness::make_template_closure_entry_context(
                witness::TemplateClosureReason::TrackInstantiation,
                entity,
                decl_location,
                true));
        witness::note_template_witness_log_event(
            witness::TemplateWitnessLogEventKind::VariableInstantiation,
            decl_location,
            entity,
            decl_location,
            std::string(),
            witness::TemplateLifecycleCause::TrackInstantiation,
            true);
      }
    }
    vector<TemplateArgument> arguments;
    TemplateArgument type_arg;
    type_arg.kind = TemplateArgument::TA_TYPE;
    type_arg.type = make_fundamental(FT_BOOL);
    type_arg.text = "bool";
    arguments.push_back(type_arg);
    TemplateArgument value_arg;
    value_arg.kind = TemplateArgument::TA_VALUE;
    value_arg.type = make_fundamental(FT_BOOL);
    value_arg.value = value ? 1 : 0;
    value_arg.text = value ? "true" : "false";
    arguments.push_back(value_arg);
    try {
      const template_api::ClassSpecializationSelection selection =
          template_api::specialization::select_class_specialization(
              *this,
              *integral_constant,
              use_scope,
              template_argument_key(arguments),
              arguments);
      const witness::ScopedTemplateWitnessSourceCapturePause pause;
      ClassInfo * info =
          instantiate_selected_class_template(*integral_constant,
                                              use_scope,
                                              arguments,
                                              selection);
      if(!info) {
        return;
      }
      MemberValueLookupResult member = lookup_member_value(*info, "value");
      if(member.binding && member.binding->kind != ValueBinding::VK_FIELD) {
        template_api::note_template_member_value_instantiation_if_needed(
            ctx,
            *member.binding);
        const std::string decl_location =
            strip_at_prefix(semantic_model::source_decl_anchor_location(
                semantic_trace::value_decl_anchor(ctx, member.binding)));
        std::string entity;
        if(integral_constant->declaring_scope &&
           scope_is_std_namespace_or_inline_child(integral_constant->declaring_scope)) {
          entity = std::string("std::integral_constant<bool, ") +
              (value ? "true" : "false") + ">::value";
        } else {
          entity = std::string("integral_constant<bool, ") +
              (value ? "true" : "false") + ">::value";
        }
        if(!decl_location.empty()) {
          const witness::ScopedTemplateWitnessEntryContext entry_context(
              witness::make_template_closure_entry_context(
                  witness::TemplateClosureReason::TrackInstantiation,
                  entity,
                  decl_location,
                  true));
          witness::note_template_witness_log_event(
              witness::TemplateWitnessLogEventKind::VariableInstantiation,
              decl_location,
              entity,
              decl_location,
              std::string(),
              witness::TemplateLifecycleCause::TrackInstantiation,
              true);
        }
      }
    } catch(const std::exception &) {
      // Witness preservation should not change constexpr viability.
    }
  }

  void track_standard_invocable_value_chain_for_witness(
      Scope & use_scope,
      const ClassTemplateDecl & class_template,
      const ClassInfo & info,
      bool structured_bool_value)
  {
    if(ctx.template_witness_context().session == nullptr ||
       !class_template.declaring_scope ||
       info.instantiation_arguments.empty()) {
      return;
    }
    static thread_local bool tracking = false;
    if(tracking) {
      return;
    }
    const bool invocable_r =
        class_template.name == "__is_invocable_r" ||
        class_template.name == "is_invocable_r";
    const bool invocable =
        class_template.name == "__is_invocable" ||
        class_template.name == "is_invocable";
    if(!invocable_r && !invocable) {
      return;
    }
    if(!scope_is_std_namespace_or_inline_child(class_template.declaring_scope)) {
      return;
    }
    if(invocable_r && info.instantiation_arguments.size() < 2) {
      return;
    }

    tracking = true;
    try {
      Scope & lookup_scope = *class_template.declaring_scope;
      if(invocable_r) {
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            class_template.name == "is_invocable_r" ?
                string("is_invocable_r_v") :
                string("__is_invocable_r_v"),
            info.instantiation_arguments);

        vector<TemplateArgument> invocable_arguments(
            info.instantiation_arguments.begin() + 1,
            info.instantiation_arguments.end());
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            class_template.name == "is_invocable_r" ?
                string("is_invocable_v") :
                string("__is_invocable_v"),
            invocable_arguments);

        vector<TemplateArgument> invocable_impl_arguments;
        TemplateArgument void_arg;
        void_arg.kind = TemplateArgument::TA_TYPE;
        void_arg.type = make_fundamental(FT_VOID);
        void_arg.text = "void";
        invocable_impl_arguments.push_back(void_arg);
        invocable_impl_arguments.insert(invocable_impl_arguments.end(),
                                        invocable_arguments.begin(),
                                        invocable_arguments.end());
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            "__is_invocable_impl",
            invocable_impl_arguments);

        vector<TemplateArgument> conversion_arguments;
        conversion_arguments.push_back(info.instantiation_arguments[0]);
        conversion_arguments.push_back(info.instantiation_arguments[0]);
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            "__is_core_convertible_v",
            conversion_arguments);
      } else {
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            class_template.name == "is_invocable" ?
                string("is_invocable_v") :
                string("__is_invocable_v"),
            info.instantiation_arguments);
        vector<TemplateArgument> invocable_impl_arguments;
        TemplateArgument void_arg;
        void_arg.kind = TemplateArgument::TA_TYPE;
        void_arg.type = make_fundamental(FT_VOID);
        void_arg.text = "void";
        invocable_impl_arguments.push_back(void_arg);
        invocable_impl_arguments.insert(invocable_impl_arguments.end(),
                                        info.instantiation_arguments.begin(),
                                        info.instantiation_arguments.end());
        track_variable_template_instantiation_for_witness(
            use_scope,
            lookup_scope,
            "__is_invocable_impl",
            invocable_impl_arguments);
      }
    } catch(const std::exception &) {
      // Witness preservation should not change constexpr viability.
    }
    try {
      Scope & lookup_scope = *class_template.declaring_scope;
      note_integral_constant_bool_value_for_witness(use_scope, lookup_scope, true);
      note_integral_constant_bool_value_for_witness(use_scope, lookup_scope, false);
      note_integral_constant_bool_value_for_witness(use_scope,
                                                   lookup_scope,
                                                   structured_bool_value);
    } catch(const std::exception &) {
      // Witness preservation should not change constexpr viability.
    }
    tracking = false;
  }

  witness::TemplateWitnessSourceAnchor class_use_selected_decl_anchor(
      ClassTemplateDecl * class_template,
      const template_api::ClassSpecializationSelection & selection) const
  {
    return callsemantic::class_use_selected_decl_anchor(
        ctx, class_template, selection);
  }

  const ValueBinding * instantiate_variable_template(
      Scope & use_scope,
      VariableTemplateDecl & decl,
      const std::vector<TemplateArgument> & arguments,
      const std::string & source_use_location)
  {
    template_api::TemplateVariableInstantiationRequest request;
    request.decl = &decl;
    request.arguments = arguments;
    request.source_use_location = source_use_location;
    request.source_use_scope = &use_scope;
    request.intent = template_api::TemplateInstantiationIntent::TrackInstantiation;
    return template_api::acquire_variable_instantiation(ctx, request).value_binding;
  }

  void finalize_class_constant_members(ClassInfo & info)
  {
    semantic_class_model::finalize_class_constant_members(ctx, info);
  }

  bool scope_is_inside_source_template_context(Scope & scope) const
  {
    return callsemantic::scope_is_inside_source_template_context(scope);
  }

  std::vector<std::string> class_template_argument_source_locations_for_current_use(
      const std::string & template_name,
      const std::vector<TemplateParameterInfo> & parameters,
      const std::vector<std::string> & arg_texts) const
  {
    return callbacks.class_template_argument_source_locations_for_current_use(
        template_name, parameters, arg_texts);
  }

  bool type_depends_on_template_parameter(const TypePtr & type) const
  {
    return ctx.type_depends_on_template_parameter(type);
  }

  bool scope_has_template_placeholders(Scope & scope) const
  {
    return ctx.scope_has_template_placeholders(scope);
  }

  std::string describe_scope_bindings_for_diagnostic(const Scope & scope) const
  {
    return ctx.describe_scope_bindings_for_diagnostic(scope);
  }

  bool evaluate_initializer_constant_value(Scope & scope,
                                           const CppAstNode & initializer,
                                           const TypePtr & target,
                                           constant_eval::ConstexprValue & value)
  {
    return ctx.evaluate_initializer_constant_value(scope, initializer, target, value);
  }

  VariableTemplateDecl * lookup_variable_template(Scope & scope,
                                                  const QualifiedName & name)
  {
    return semantic_lookup::lookup_variable_template(ctx, scope, name);
  }

  bool resolve_template_arguments(Scope & scope,
                                  const std::vector<TemplateParameterInfo> & parameters,
                                  const std::vector<std::string> & arg_texts,
                                  const std::vector<TemplateArgumentSyntax> * arg_syntaxes,
                                  std::vector<TemplateArgument> & out,
                                  Scope * default_argument_declaring_scope)
  {
    return template_api::resolve_template_arguments(ctx,
                                                    scope,
                                                    parameters,
                                                    arg_texts,
                                                    arg_syntaxes,
                                                    out,
                                                    default_argument_declaring_scope);
  }

  std::vector<FunctionTemplateDecl *> lookup_function_templates(Scope & scope,
                                                                const std::string & name)
  {
    return ctx.lookup_function_templates(scope, name);
  }

  std::vector<FunctionTemplateDecl *> lookup_function_templates(Scope & scope,
                                                                const QualifiedName & name)
  {
    return ctx.lookup_qualified_function_templates(scope, name);
  }

  ClassTemplateDecl * lookup_class_template(Scope & scope,
                                            const std::string & name)
  {
    return ctx.lookup_class_template(scope, name);
  }

  ClassTemplateDecl * lookup_class_template(Scope & scope,
                                            const QualifiedName & name)
  {
    return ctx.lookup_class_template(scope, name);
  }

  ClassInfo * instantiate_selected_class_template(
      ClassTemplateDecl & decl,
      Scope & use_scope,
      const std::vector<TemplateArgument> & arguments,
      const template_api::ClassSpecializationSelection & selection)
  {
    return ctx.instantiate_selected_class_template(decl, use_scope, arguments, selection);
  }

  TypePtr lookup_type(Scope & scope,
                      const std::string & name,
                      bool reference_class_templates_only = false)
  {
    return ctx.lookup_type(scope, name, reference_class_templates_only);
  }

  ClassInfo * complete_class_type(const TypePtr & type)
  {
    return ctx.complete_class_type(type);
  }

  ClassInfo * class_info_for_type(const TypePtr & type) const
  {
    return ctx.class_info_for_type(type);
  }

  const ValueBinding * lookup_value(Scope & scope,
                                    const std::string & name)
  {
    return ctx.lookup_value(scope, name);
  }
};

}  // namespace


bool materialize_constant_binding_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    ValueBinding & binding,
    constant_eval::ConstexprValue & value)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.materialize_constant_binding_value(binding, value);
}

bool lookup_constant_template_id_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const TemplateIdSyntax & template_id,
    const std::string & display_name,
    constant_eval::ConstexprValue & out)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.lookup_constant_template_id_value(scope, template_id, display_name, out);
}

void record_constexpr_direct_function_call_source_use(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const CppAstNode & callee,
    FunctionBinding & binding,
    const TemplateIdSyntax * template_id_syntax,
    std::size_t explicit_arg_count)
{
  ConstantValueLookup lookup(ctx, callbacks);
  lookup.record_constexpr_direct_function_call_source_use(
      scope, callee, binding, template_id_syntax, explicit_arg_count);
}

bool lookup_constant_template_member_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const TemplateIdSyntax & qualifier_template_id,
    const std::string & member_name,
    const std::string & display_name,
    constant_eval::ConstexprValue & out)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.lookup_constant_template_member_value(
      scope, qualifier_template_id, member_name, display_name, out);
}

bool lookup_constant_value_node(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const std::string & name,
    const CppAstNode * node,
    constant_eval::ConstexprValue & out)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.lookup_constant_value_node(scope, name, node, out);
}

bool lookup_constant_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const std::string & name,
    constant_eval::ConstexprValue & out)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.lookup_constant_value(scope, name, out);
}

bool lookup_constant_value(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const std::string & name,
    long long & out)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.lookup_constant_value(scope, name, out);
}

Scope * resolve_qualified_scope_for_node(
    SemanticContext & ctx,
    const ConstantValueLookupCallbacks & callbacks,
    Scope & scope,
    const QualifiedName & qualified,
    const CppAstNode & node,
    bool allow_dependent_class_qualifiers)
{
  ConstantValueLookup lookup(ctx, callbacks);
  return lookup.resolve_qualified_scope_for_node(
      scope, qualified, node, allow_dependent_class_qualifiers);
}

}  // namespace callsemantic
