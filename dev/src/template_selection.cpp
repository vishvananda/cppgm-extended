#include "template_selection.h"

#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "class_template_mangle_info.h"
#include "cpp_decl_model.h"
#include "parser_trace.h"
#include "template_api.h"
#include "template_argument_semantics.h"
#include "template_specialization.h"
#include "template_witness.h"

namespace template_selection {

using namespace semantic_model;
using namespace template_model;

namespace {

std::string join_partial_arg_texts(const std::vector<std::string> & arg_texts)
{
  std::ostringstream out;
  for(std::size_t i = 0; i < arg_texts.size(); ++i) {
    if(i != 0) {
      out << ", ";
    }
    out << arg_texts[i];
  }
  return out.str();
}

typedef std::pair<const void *, std::string> SpecializationSelectionKey;

thread_local std::set<SpecializationSelectionKey> class_selection_in_progress;
thread_local std::set<SpecializationSelectionKey> variable_selection_in_progress;
thread_local std::size_t class_reentrant_primary_selection_count = 0;

template<class Set>
struct ScopedSelectionGuard
{
  ScopedSelectionGuard(Set & set, const SpecializationSelectionKey & key)
      : set(set),
        key(key),
        inserted(set.insert(key).second)
  {}

  ~ScopedSelectionGuard()
  {
    if(inserted) {
      set.erase(key);
    }
  }

  Set & set;
  SpecializationSelectionKey key;
  bool inserted;
};

template<typename PartialDecl, typename MatchFn, typename CompareFn>
const PartialDecl * select_best_partial_specialization(
    const std::vector<PartialDecl> & partial_specializations,
    const MatchFn & match,
    const CompareFn & compare,
    std::vector<TemplateArgument> & chosen_arguments,
    std::map<std::string, std::size_t> & chosen_pack_sizes,
    std::size_t & chosen_score,
    const char * ambiguous_message)
{
  struct Candidate
  {
    const PartialDecl * partial = nullptr;
    std::vector<TemplateArgument> arguments;
    std::map<std::string, std::size_t> pack_sizes;
    std::size_t score = 0;
  };

  std::vector<Candidate> candidates;
  std::size_t best_score = 0;
  bool have_candidate = false;
  chosen_score = 0;
  chosen_pack_sizes.clear();

  for(std::size_t i = 0; i < partial_specializations.size(); ++i) {
    std::vector<TemplateArgument> partial_arguments;
    std::map<std::string, std::size_t> partial_pack_sizes;
    std::size_t partial_score = 0;
    if(!match(partial_specializations[i], partial_arguments, partial_pack_sizes, partial_score)) {
      continue;
    }
    if(!have_candidate || partial_score > best_score) {
      candidates.clear();
      best_score = partial_score;
      have_candidate = true;
    }
    if(partial_score == best_score) {
      Candidate candidate;
      candidate.partial = &partial_specializations[i];
      candidate.arguments = partial_arguments;
      candidate.pack_sizes = partial_pack_sizes;
      candidate.score = partial_score;
      candidates.push_back(candidate);
    }
  }

  if(candidates.empty()) {
    return nullptr;
  }

  std::size_t winner_index = candidates.size();
  for(std::size_t i = 0; i < candidates.size(); ++i) {
    bool more_specialized_than_all = true;
    for(std::size_t j = 0; j < candidates.size(); ++j) {
      if(i == j) {
        continue;
      }
      if(compare(*candidates[i].partial, *candidates[j].partial) >= 0) {
        more_specialized_than_all = false;
        break;
      }
    }
    if(more_specialized_than_all) {
      if(winner_index != candidates.size()) {
        throw std::logic_error(ambiguous_message);
      }
      winner_index = i;
    }
  }

  if(winner_index == candidates.size()) {
    throw std::logic_error(ambiguous_message);
  }

  chosen_arguments = candidates[winner_index].arguments;
  chosen_pack_sizes = candidates[winner_index].pack_sizes;
  chosen_score = candidates[winner_index].score;
  return candidates[winner_index].partial;
}

ClassSpecializationSelection make_primary_class_selection(
    ClassTemplateDecl & decl,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments)
{
  ClassSpecializationSelection selection;
  selection.class_node = decl.class_node;
  selection.binding_scope = decl.declaring_scope;
  selection.parameters = &decl.parameters;
  selection.arguments = arguments;
  selection.selection_key = key;
  return selection;
}

const std::vector<TemplateParameterInfo> & empty_template_parameters()
{
  static const std::vector<TemplateParameterInfo> parameters;
  return parameters;
}

const PartialClassTemplateSpecializationDecl *
select_exact_dependent_partial_class_specialization(
    ClassTemplateDecl & decl,
    const std::vector<std::string> & source_argument_texts)
{
  const PartialClassTemplateSpecializationDecl * exact_partial = nullptr;
  for(std::size_t i = 0; i < decl.partial_specializations.size(); ++i) {
    const PartialClassTemplateSpecializationDecl & partial =
        decl.partial_specializations[i];
    if(partial.arg_texts.size() != source_argument_texts.size() ||
       !partial.class_node) {
      continue;
    }

    bool matches = true;
    for(std::size_t j = 0; j < source_argument_texts.size(); ++j) {
      if(!template_argument_semantics::normalized_type_lookup_text_matches(
             partial.arg_texts[j],
             source_argument_texts[j])) {
        matches = false;
        break;
      }
    }
    if(!matches) {
      continue;
    }
    if(exact_partial != nullptr) {
      return nullptr;
    }
    exact_partial = &partial;
  }
  return exact_partial;
}

bool apply_exact_dependent_partial_class_selection(
    ClassSpecializationSelection & selection,
    ClassTemplateDecl & decl,
    const std::string & key,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  if(!(dependent_source_argument_texts && !dependent_source_argument_texts->empty())) {
    return false;
  }
  const PartialClassTemplateSpecializationDecl * exact_partial =
      select_exact_dependent_partial_class_specialization(
          decl, *dependent_source_argument_texts);
  if(!exact_partial) {
    return false;
  }

  selection.class_node = exact_partial->class_node;
  selection.binding_scope =
      exact_partial->pattern_scope ? exact_partial->pattern_scope :
                                     exact_partial->declaring_scope;
  selection.parameters = &empty_template_parameters();
  selection.arguments.clear();
  if(exact_partial->arg_syntaxes.size() == dependent_source_argument_texts->size()) {
    selection.argument_syntaxes = &exact_partial->arg_syntaxes;
  }
  selection.pack_sizes.clear();
  selection.selection_key.clear();
  selection.kind = MS_PARTIAL_SPECIALIZATION;
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "class-specialization name=" << decl.name
          << " kind=exact-dependent-partial"
          << " key=" << key
          << " pattern=" << join_partial_arg_texts(exact_partial->arg_texts);
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return true;
}

void apply_explicit_class_selection(
    ClassSpecializationSelection & selection,
    const ClassTemplateSpecializationDecl & specialization)
{
  selection.class_node = specialization.class_node;
  selection.binding_scope = specialization.declaring_scope;
  selection.kind = MS_EXPLICIT_SPECIALIZATION;
}

void apply_partial_class_selection(
    ClassSpecializationSelection & selection,
    const PartialClassTemplateSpecializationDecl & specialization,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> & pack_sizes)
{
  selection.class_node = specialization.class_node;
  selection.binding_scope =
      specialization.pattern_scope ? specialization.pattern_scope :
                                     specialization.declaring_scope;
  selection.parameters = &specialization.parameters;
  selection.arguments = arguments;
  selection.pack_sizes = pack_sizes;
  selection.kind = MS_PARTIAL_SPECIALIZATION;
}

bool class_specialization_selection_cache_enabled(
    template_api::TemplateServices & services,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  return services.semantic_context &&
         services.witness_context.session == nullptr &&
         !template_api::semantic_source_use_capture_enabled() &&
         !parser_trace::enabled("template.resolve") &&
         !(dependent_source_argument_texts && !dependent_source_argument_texts->empty()) &&
         !template_api::template_arguments_are_dependent(
             *services.semantic_context,
             arguments);
}

bool find_cached_class_specialization_selection(
    const ClassTemplateDecl & decl,
    const std::string & key,
    ClassSpecializationSelection & selection)
{
  std::map<std::string,
           ClassTemplateDecl::SpecializationSelectionCacheEntry>::const_iterator found =
      decl.specialization_selection_cache.find(key);
  if(found == decl.specialization_selection_cache.end() ||
     found->second.specialization_epoch != decl.specialization_epoch) {
    return false;
  }

  selection.class_node = found->second.class_node;
  selection.binding_scope = found->second.binding_scope;
  selection.parameters = found->second.parameters;
  selection.arguments = found->second.arguments;
  selection.pack_sizes = found->second.pack_sizes;
  selection.selection_key = found->second.selection_key;
  selection.kind = static_cast<MatchKind>(found->second.kind);
  return true;
}

void cache_class_specialization_selection(
    ClassTemplateDecl & decl,
    const std::string & key,
    const ClassSpecializationSelection & selection)
{
  ClassTemplateDecl::SpecializationSelectionCacheEntry entry;
  entry.specialization_epoch = decl.specialization_epoch;
  entry.class_node = selection.class_node;
  entry.binding_scope = selection.binding_scope;
  entry.parameters = selection.parameters;
  entry.arguments = selection.arguments;
  entry.pack_sizes = selection.pack_sizes;
  entry.selection_key = selection.selection_key;
  entry.kind = static_cast<int>(selection.kind);
  decl.specialization_selection_cache[key] = entry;
}

bool type_has_class_template_metadata(const cpp_decl::TypePtr & type);

bool template_argument_has_class_template_metadata(const TemplateArgument & argument)
{
  if(argument.kind == TemplateArgument::TA_TYPE) {
    return type_has_class_template_metadata(argument.type);
  }
  if(argument.kind == TemplateArgument::TA_VALUE) {
    return type_has_class_template_metadata(argument.type);
  }
  return false;
}

bool type_has_class_template_metadata(const cpp_decl::TypePtr & type)
{
  cpp_decl::TypePtr base = cpp_decl::strip_top_level_cv(type);
  if(!base || base->kind != cpp_decl::Type::TK_NAMED) {
    return false;
  }
  if(cpp_decl::named_type_class_template_specialization_mangle_info_const(base)) {
    return true;
  }
  void * class_template_decl = nullptr;
  std::vector<cpp_decl::DependentAliasTemplateArgumentSyntax> arguments;
  return cpp_decl::named_type_dependent_class_template(base,
                                                      class_template_decl,
                                                      arguments);
}

bool class_primary_selection_cache_safe(
    const std::vector<TemplateArgument> & arguments)
{
  for(std::size_t i = 0; i < arguments.size(); ++i) {
    if(template_argument_has_class_template_metadata(arguments[i])) {
      return false;
    }
  }
  return true;
}

VariableSpecializationSelection make_primary_variable_selection(
    VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments)
{
  VariableSpecializationSelection selection;
  selection.binding_scope = decl.declaring_scope;
  selection.parameters = &decl.parameters;
  selection.arguments = arguments;
  selection.specifiers = decl.specifiers;
  selection.declarator = decl.declarator;
  selection.initializer = decl.initializer;
  selection.selection_key = key;
  return selection;
}

void apply_explicit_variable_selection(
    VariableSpecializationSelection & selection,
    const VariableTemplateSpecializationDecl & specialization)
{
  selection.binding_scope = specialization.declaring_scope;
  selection.specifiers = specialization.specifiers;
  selection.declarator = specialization.declarator;
  selection.initializer = specialization.initializer;
  selection.kind = MS_EXPLICIT_SPECIALIZATION;
}

void apply_partial_variable_selection(
    VariableSpecializationSelection & selection,
    const VariableTemplateSpecializationDecl & specialization,
    const std::vector<TemplateArgument> & arguments,
    const std::map<std::string, std::size_t> & pack_sizes)
{
  selection.binding_scope = specialization.declaring_scope;
  selection.parameters = &specialization.parameters;
  selection.arguments = arguments;
  selection.pack_sizes = pack_sizes;
  selection.specifiers = specialization.specifiers;
  selection.declarator = specialization.declarator;
  selection.initializer = specialization.initializer;
  selection.kind = MS_PARTIAL_SPECIALIZATION;
}

void replay_alias_template_id_value_dependencies(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax);

void replay_alias_template_id_value_dependencies(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateIdSyntax & syntax)
{
  for(std::size_t i = 0; i < syntax.argument_syntaxes.size(); ++i) {
    const cpp_decl::TemplateArgumentSyntax & argument =
        syntax.argument_syntaxes[i];
    replay_alias_template_id_value_dependencies(services, scope, argument);
  }
}

void replay_alias_template_id_value_dependencies(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle scope,
    const cpp_decl::TemplateArgumentSyntax & syntax)
{
  if(syntax.template_id) {
    std::vector<std::string> expansion_arg_texts =
        syntax.template_id->arguments;
    const std::vector<cpp_decl::TemplateArgumentSyntax> * expansion_arg_syntaxes =
        &syntax.template_id->argument_syntaxes;
    semantic_model::AliasTemplateDecl * alias_template = nullptr;
    std::vector<TemplateArgument> resolved_arguments;
    if(services.semantic_context) {
      alias_template =
          template_argument_semantics::lookup_alias_template(
              services,
              scope.require(),
              syntax.template_id->name.name);
      if(alias_template &&
         template_api::resolve_template_arguments(
             *services.semantic_context,
             scope.require(),
             alias_template->parameters,
             syntax.template_id->arguments,
             &syntax.template_id->argument_syntaxes,
             resolved_arguments,
             alias_template->declaring_scope) &&
         !template_api::template_arguments_are_dependent(
             *services.semantic_context,
             resolved_arguments)) {
        expansion_arg_texts =
            template_api::canonical_template_argument_texts(
                *services.semantic_context,
                resolved_arguments);
        expansion_arg_syntaxes = nullptr;
        if(alias_template->declaring_scope && alias_template->type_id) {
          Scope & alias_scope =
              template_api::binding::bind_template_arguments(
                  *services.semantic_context,
                  *alias_template->declaring_scope,
                  alias_template->parameters,
                  resolved_arguments);
          cpp_decl::TemplateArgumentSyntax alias_type_syntax;
          alias_type_syntax.type_id.reset(new CppAstNode(*alias_template->type_id));
          std::vector<TemplateValueDependency> dependencies;
          template_argument_semantics::
              append_structured_bool_value_dependencies_in_template_argument_syntax(
                  services,
                  template_api::make_template_environment(alias_scope),
                  alias_type_syntax,
                  dependencies);
          template_argument_semantics::note_template_value_dependencies_for_witness(
              *services.semantic_context,
              dependencies);
          template_argument_semantics::
              note_structured_bool_value_members_in_template_argument_syntax(
                  services,
                  template_api::make_template_environment(alias_scope),
                  alias_type_syntax);
        }
      }
    }
    std::string expanded_text;
    const bool expanded = template_specialization::expand_alias_template_pattern_id(
        services,
        scope,
        syntax.text,
        syntax.template_id->name,
        expansion_arg_texts,
        expanded_text,
        expansion_arg_syntaxes,
        scope);
    (void)expanded;
    replay_alias_template_id_value_dependencies(
        services,
        scope,
        *syntax.template_id);
  }
}

void replay_selected_partial_class_value_dependencies(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle use_scope,
    const PartialClassTemplateSpecializationDecl & partial,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<TemplateArgument> & partial_arguments,
    const std::map<std::string, std::size_t> & partial_pack_sizes)
{
  if(services.witness_context.session == nullptr ||
     !services.semantic_context ||
     template_api::template_arguments_are_dependent(*services.semantic_context,
                                                    arguments) ||
     template_api::template_arguments_are_dependent(*services.semantic_context,
                                                    partial_arguments) ||
     !(partial.pattern_scope || partial.declaring_scope)) {
    return;
  }

  try {
    const template_api::ScopedTemplateWitnessSourceCapturePause source_capture_pause;
    bool has_source_defaulted_value_argument = false;
    for(std::size_t i = 0; i < arguments.size(); ++i) {
      if(arguments[i].kind == TemplateArgument::TA_VALUE &&
         arguments[i].source_defaulted) {
        has_source_defaulted_value_argument = true;
        break;
      }
    }
    Scope & partial_scope =
        partial.pattern_scope ? *partial.pattern_scope : *partial.declaring_scope;
    Scope & replay_scope =
        template_api::binding::bind_template_arguments(
            *services.semantic_context,
            partial_scope,
            partial.parameters,
            partial_arguments,
            partial_pack_sizes.empty() ? nullptr : &partial_pack_sizes);
    for(std::size_t i = 0; i < partial.arg_syntaxes.size(); ++i) {
      replay_alias_template_id_value_dependencies(
          services,
          template_api::make_template_environment(replay_scope),
          partial.arg_syntaxes[i]);
    }
    if(has_source_defaulted_value_argument) {
      return;
    }
    std::vector<TemplateArgument> replay_arguments;
    std::map<std::string, std::size_t> replay_pack_sizes;
    std::size_t replay_score = 0;
    (void)template_specialization::match_partial_class_specialization(
        services,
        use_scope,
        partial,
        arguments,
        replay_arguments,
        replay_score,
        &replay_pack_sizes);
  } catch(const std::exception &) {
  }
}

void replay_concrete_class_value_dependencies(
    template_api::TemplateServices & services,
    template_api::TemplateEnvironmentHandle use_scope,
    const std::vector<TemplateArgument> & arguments)
{
  if(services.witness_context.session == nullptr ||
     !services.semantic_context ||
     template_api::template_arguments_are_dependent(*services.semantic_context,
                                                    arguments)) {
    return;
  }
  template_argument_semantics::note_structured_bool_value_members_in_template_arguments(
      services,
      use_scope,
      arguments);
}

}  // namespace

ClassSpecializationSelection select_class_specialization(
    template_api::TemplateServices & services,
    ClassTemplateDecl & decl,
    template_api::TemplateEnvironmentHandle use_scope,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments,
    const std::vector<std::string> * dependent_source_argument_texts)
{
  ClassSpecializationSelection selection =
      make_primary_class_selection(decl, key, arguments);
  const bool use_selection_cache =
      class_specialization_selection_cache_enabled(
          services,
          arguments,
          dependent_source_argument_texts);
  if(use_selection_cache &&
     find_cached_class_specialization_selection(decl, key, selection)) {
    return selection;
  }

  if(apply_exact_dependent_partial_class_selection(
         selection, decl, key, dependent_source_argument_texts)) {
    return selection;
  }

  std::map<std::string, ClassTemplateSpecializationDecl>::iterator explicit_found =
      decl.explicit_specializations.find(key);
  if(explicit_found != decl.explicit_specializations.end()) {
    apply_explicit_class_selection(selection, explicit_found->second);
    replay_concrete_class_value_dependencies(services, use_scope, arguments);
    if(use_selection_cache) {
      cache_class_specialization_selection(decl, key, selection);
    }
    return selection;
  }

  if(decl.partial_specializations.empty()) {
    replay_concrete_class_value_dependencies(services, use_scope, arguments);
    if(use_selection_cache) {
      cache_class_specialization_selection(decl, key, selection);
    }
    return selection;
  }

  ScopedSelectionGuard<std::set<SpecializationSelectionKey> > guard(
      class_selection_in_progress,
      SpecializationSelectionKey(&decl, key));
  if(!guard.inserted) {
    selection.reentrant_primary = true;
    ++class_reentrant_primary_selection_count;
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "class-specialization name=" << decl.name
            << " kind=reentrant-primary"
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return selection;
  }
  const std::size_t reentrant_primary_count_before =
      class_reentrant_primary_selection_count;

  std::vector<TemplateArgument> partial_arguments;
  std::map<std::string, std::size_t> partial_pack_sizes;
  std::size_t partial_score = 0;
  bool partial_match_deferred = false;
  const PartialClassTemplateSpecializationDecl * chosen_partial =
      select_best_partial_specialization(
          decl.partial_specializations,
          [&](const PartialClassTemplateSpecializationDecl & partial,
              std::vector<TemplateArgument> & deduced_arguments,
              std::map<std::string, std::size_t> & deduced_pack_sizes,
              std::size_t & specificity_score) -> bool
          {
            const template_api::ScopedTemplateWitnessSourceCapturePause
                source_capture_pause;
            const template_api::ScopedTemplateWitnessLifecyclePause
                lifecycle_pause;
            bool candidate_deferred = false;
            const bool matched =
                template_specialization::match_partial_class_specialization(services,
                                                                            use_scope,
                                                                            partial,
                                                                            arguments,
                                                                            deduced_arguments,
                                                                            specificity_score,
                                                                            &deduced_pack_sizes,
                                                                            &candidate_deferred);
            if(candidate_deferred) {
              partial_match_deferred = true;
            }
            if(parser_trace::enabled("template.resolve")) {
              std::ostringstream trace;
              trace << "class-specialization-candidate name=" << decl.name
                    << " matched=" << (matched ? "yes" : "no")
                    << " deferred=" << (candidate_deferred ? "yes" : "no")
                    << " score=" << specificity_score
                    << " pattern=" << join_partial_arg_texts(partial.arg_texts)
                    << " key=" << key;
              parser_trace::note("template.resolve", std::string(), trace.str());
            }
            return matched;
          },
          [&](const PartialClassTemplateSpecializationDecl & current,
              const PartialClassTemplateSpecializationDecl & best) -> int
          {
            return template_specialization::compare_partial_class_specialization_preference(
                services, current, best);
          },
          partial_arguments,
          partial_pack_sizes,
          partial_score,
          "ambiguous partial class specialization");
  const bool dependent_selection_arguments =
      services.semantic_context &&
      template_api::template_arguments_are_dependent(*services.semantic_context,
                                                     arguments);
  const bool selection_deferred =
      partial_match_deferred &&
      (!chosen_partial || dependent_selection_arguments);
  if(selection_deferred) {
    chosen_partial = nullptr;
  }
  const bool saw_reentrant_primary_selection =
      class_reentrant_primary_selection_count != reentrant_primary_count_before;
  if(!chosen_partial) {
    if(selection_deferred) {
      selection.reentrant_primary = true;
    }
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "class-specialization name=" << decl.name
            << " kind=" << (selection_deferred ? "deferred-primary" : "primary")
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    if(!selection_deferred) {
      replay_concrete_class_value_dependencies(services, use_scope, arguments);
    }
    if(use_selection_cache &&
       !selection_deferred &&
       !saw_reentrant_primary_selection &&
       class_primary_selection_cache_safe(arguments)) {
      cache_class_specialization_selection(decl, key, selection);
    }
    return selection;
  }

  apply_partial_class_selection(selection,
                                *chosen_partial,
                                partial_arguments,
                                partial_pack_sizes);
  replay_selected_partial_class_value_dependencies(
      services,
      use_scope,
      *chosen_partial,
      arguments,
      partial_arguments,
      partial_pack_sizes);
  replay_concrete_class_value_dependencies(services, use_scope, arguments);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "class-specialization name=" << decl.name
          << " kind=partial"
          << " score=" << partial_score
          << " key=" << key
          << " pattern=" << join_partial_arg_texts(chosen_partial->arg_texts);
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  if(use_selection_cache && !saw_reentrant_primary_selection) {
    cache_class_specialization_selection(decl, key, selection);
  }
  return selection;
}

VariableSpecializationSelection select_variable_specialization(
    template_api::TemplateServices & services,
    VariableTemplateDecl & decl,
    const std::string & key,
    const std::vector<TemplateArgument> & arguments)
{
  VariableSpecializationSelection selection =
      make_primary_variable_selection(decl, key, arguments);

  std::map<std::string, VariableTemplateSpecializationDecl>::iterator explicit_found =
      decl.explicit_specializations.find(key);
  if(explicit_found != decl.explicit_specializations.end()) {
    apply_explicit_variable_selection(selection, explicit_found->second);
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "variable-specialization name=" << decl.name
            << " kind=explicit"
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return selection;
  }

  if(decl.partial_specializations.empty()) {
    return selection;
  }

  ScopedSelectionGuard<std::set<SpecializationSelectionKey> > guard(
      variable_selection_in_progress,
      SpecializationSelectionKey(&decl, key));
  if(!guard.inserted) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "variable-specialization name=" << decl.name
            << " kind=reentrant-primary"
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return selection;
  }

  std::vector<TemplateArgument> partial_arguments;
  std::map<std::string, std::size_t> partial_pack_sizes;
  std::size_t partial_score = 0;
  const VariableTemplateSpecializationDecl * chosen_partial =
      select_best_partial_specialization(
          decl.partial_specializations,
          [&](const VariableTemplateSpecializationDecl & partial,
              std::vector<TemplateArgument> & deduced_arguments,
              std::map<std::string, std::size_t> & deduced_pack_sizes,
              std::size_t & specificity_score) -> bool
          {
            const template_api::ScopedTemplateWitnessSourceCapturePause
                source_capture_pause;
            const template_api::ScopedTemplateWitnessLifecyclePause
                lifecycle_pause;
            return template_specialization::match_partial_variable_specialization(
                services,
                template_api::make_template_environment(*decl.declaring_scope),
                partial,
                arguments,
                deduced_arguments,
                specificity_score,
                &deduced_pack_sizes);
          },
          [&](const VariableTemplateSpecializationDecl & current,
              const VariableTemplateSpecializationDecl & best) -> int
          {
            return template_specialization::compare_partial_variable_specialization_preference(
                services, current, best);
          },
          partial_arguments,
          partial_pack_sizes,
          partial_score,
          "ambiguous partial variable specialization");
  if(!chosen_partial) {
    if(parser_trace::enabled("template.resolve")) {
      std::ostringstream trace;
      trace << "variable-specialization name=" << decl.name
            << " kind=primary"
            << " key=" << key;
      parser_trace::note("template.resolve", std::string(), trace.str());
    }
    return selection;
  }

  apply_partial_variable_selection(selection,
                                   *chosen_partial,
                                   partial_arguments,
                                   partial_pack_sizes);
  if(parser_trace::enabled("template.resolve")) {
    std::ostringstream trace;
    trace << "variable-specialization name=" << decl.name
          << " kind=partial"
          << " score=" << partial_score
          << " key=" << key;
    parser_trace::note("template.resolve", std::string(), trace.str());
  }
  return selection;
}

}  // namespace template_selection
