#include "callsemantic/function_registry.h"

#include <algorithm>
#include <map>
#include <stdexcept>

#include "semantic_lookup.h"
#include "template_api.h"

namespace callsemantic {

using cpp_decl::Type;
using cpp_decl::TypePtr;
using semantic_model::ClassInfo;
using semantic_model::FunctionBinding;
using semantic_model::FunctionTemplateDecl;
using semantic_model::RefQualifier;
using semantic_model::RQ_NONE;
using semantic_model::Scope;
using template_model::TemplateArgument;

namespace {

int binding_score(const FunctionBinding * binding)
{
  if(!binding) {
    return -1;
  }

  int score = 0;
  if(binding->has_definition) {
    score += 100;
  }
  if(binding->definition_node || binding->body) {
    score += 50;
  } else if(binding->declaration_node) {
    score += 10;
  }
  if(binding->output_emitted) {
    score += 20;
  }
  if(binding->definition_output_emitted) {
    score += 20;
  }
  if(binding->synthesized || binding->is_defaulted) {
    score += 25;
  }
  if(binding->source_template) {
    score += 10;
  }
  if(binding->owner_class && binding->owner_class->source_template) {
    score += 5;
  }
  if(has_output_requirement(binding->output_requirements,
                            semantic_model::ORK_DEFINITION)) {
    score += 1;
  }
  return score;
}

void erase_function_pointer(std::vector<FunctionBinding *> & bindings,
                            FunctionBinding * binding)
{
  bindings.erase(std::remove(bindings.begin(), bindings.end(), binding),
                 bindings.end());
}

void erase_function_pointer_map(std::map<std::string, std::vector<FunctionBinding *> > & bindings,
                                FunctionBinding * binding)
{
  for(std::map<std::string, std::vector<FunctionBinding *> >::iterator it = bindings.begin();
      it != bindings.end();) {
    erase_function_pointer(it->second, binding);
    if(it->second.empty()) {
      bindings.erase(it++);
    } else {
      ++it;
    }
  }
}

void erase_function_binding_from_owner_lookups(FunctionBinding * binding)
{
  if(!binding) {
    return;
  }
  if(binding->owner_class) {
    erase_function_pointer_map(binding->owner_class->methods, binding);
    if(binding->owner_class->member_scope) {
      erase_function_pointer_map(binding->owner_class->member_scope->function_sets, binding);
    }
  }
  if(binding->declaration_scope) {
    erase_function_pointer_map(binding->declaration_scope->function_sets, binding);
  }
}

bool internal_symbol_has_other_function_owner(
    const FunctionRegistryState & state,
    const std::string & internal_symbol,
    const FunctionBinding * ignored)
{
  if(internal_symbol.empty()) {
    return false;
  }
  std::unordered_map<std::string, std::vector<FunctionBinding *> >::const_iterator
      found = state.functions_by_internal_symbol.find(internal_symbol);
  if(found == state.functions_by_internal_symbol.end()) {
    return false;
  }
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    const FunctionBinding * candidate = found->second[i];
    if(candidate && candidate != ignored) {
      return true;
    }
  }
  return false;
}

bool function_binding_belongs_to_class(const FunctionBinding * binding,
                                       const ClassInfo & info)
{
  if(!binding) {
    return false;
  }
  if(binding->owner_class == &info) {
    return true;
  }
  return info.member_scope &&
         binding->declaration_scope == info.member_scope.get();
}

}  // namespace

FunctionBinding * find_function_by_symbol(
    const FunctionRegistryState & state,
    const FunctionRegistryCallbacks & callbacks,
    const symbol_linkage::SymbolIdentity & symbol,
    const std::string & name,
    const TypePtr & type)
{
  if(!symbol.internal_symbol.empty()) {
    FunctionBinding * best_symbol_match = nullptr;
    int best_symbol_score = -1;
    std::unordered_map<std::string, std::vector<FunctionBinding *> >::const_iterator
        found = state.functions_by_internal_symbol.find(symbol.internal_symbol);
    if(found != state.functions_by_internal_symbol.end()) {
      for(std::size_t i = 0; i < found->second.size(); ++i) {
        FunctionBinding * candidate = found->second[i];
        if(!candidate) {
          continue;
        }
        const bool signature_match =
            candidate->name == name &&
            callbacks.types_equivalent_for_member_binding(candidate->type, type);
        if(!signature_match) {
          continue;
        }
        const int score = binding_score(candidate);
        if(score > best_symbol_score) {
          best_symbol_match = candidate;
          best_symbol_score = score;
        }
      }
    }
    if(best_symbol_match) {
      return best_symbol_match;
    }
  }

  FunctionBinding * best = nullptr;
  int best_score = -1;
  for(std::size_t i = 0; i < state.functions.size(); ++i) {
    if(!state.functions[i]) {
      continue;
    }
    const bool signature_match =
        state.functions[i]->name == name &&
        callbacks.types_equivalent_for_member_binding(state.functions[i]->type,
                                                      type);
    if(!signature_match) {
      continue;
    }
    int score = binding_score(state.functions[i].get()) + 10;
    if(score > best_score) {
      best = state.functions[i].get();
      best_score = score;
    }
  }
  return best;
}

bool types_equivalent_for_member_binding(const TypePtr & lhs,
                                         const TypePtr & rhs)
{
  if(lhs.get() == rhs.get()) {
    return true;
  }
  if(!lhs || !rhs || lhs->kind != rhs->kind) {
    return false;
  }

  switch(lhs->kind) {
  case Type::TK_FUNDAMENTAL:
    return lhs->fundamental == rhs->fundamental;

  case Type::TK_NAMED:
    return lhs->named_key == rhs->named_key;

  case Type::TK_CV:
    return lhs->cv_const == rhs->cv_const &&
           lhs->cv_volatile == rhs->cv_volatile &&
           types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_ATOMIC:
    return types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_POINTER:
  case Type::TK_BLOCK_POINTER:
    return types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_LVALUE_REFERENCE:
  case Type::TK_RVALUE_REFERENCE:
    return lhs->kind == rhs->kind &&
           types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_MEMBER_POINTER:
    return types_equivalent_for_member_binding(lhs->owner, rhs->owner) &&
           types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_ARRAY:
    return lhs->has_bound == rhs->has_bound &&
           lhs->bound == rhs->bound &&
           lhs->bound_text == rhs->bound_text &&
           types_equivalent_for_member_binding(lhs->inner, rhs->inner);

  case Type::TK_FUNCTION:
    if(lhs->variadic != rhs->variadic ||
       lhs->prototype_relaxed != rhs->prototype_relaxed ||
       lhs->function_const != rhs->function_const ||
       lhs->function_volatile != rhs->function_volatile ||
       lhs->params.size() != rhs->params.size() ||
       !types_equivalent_for_member_binding(lhs->inner, rhs->inner)) {
      return false;
    }
    for(std::size_t i = 0; i < lhs->params.size(); ++i) {
      if(!types_equivalent_for_member_binding(lhs->params[i], rhs->params[i])) {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool function_types_equivalent_for_member_signature(const TypePtr & lhs,
                                                    const TypePtr & rhs)
{
  if(lhs.get() == rhs.get()) {
    return true;
  }
  if(!lhs || !rhs ||
     lhs->kind != Type::TK_FUNCTION ||
     rhs->kind != Type::TK_FUNCTION) {
    return false;
  }

  if(lhs->variadic != rhs->variadic ||
     lhs->prototype_relaxed != rhs->prototype_relaxed ||
     lhs->function_const != rhs->function_const ||
     lhs->function_volatile != rhs->function_volatile ||
     lhs->params.size() != rhs->params.size()) {
    return false;
  }
  for(std::size_t i = 0; i < lhs->params.size(); ++i) {
    if(!types_equivalent_for_member_binding(lhs->params[i], rhs->params[i])) {
      return false;
    }
  }
  return true;
}

bool function_binding_matches_materialized_owner_template_identity(
    const FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key)
{
  FunctionTemplateRegistrationIdentity identity;
  identity.decl = source_template;
  identity.key = instantiation_key;
  return template_api::function_binding_matches_materialized_owner_template_identity(
      binding, identity);
}

bool function_binding_matches_instantiation_identity(
    const FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key)
{
  FunctionTemplateRegistrationIdentity identity;
  identity.decl = source_template;
  identity.key = instantiation_key;
  return template_api::function_binding_matches_instantiation_identity(
      binding, identity);
}

void maybe_adopt_materialized_owner_template_identity(
    FunctionBinding & binding,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    const std::vector<TemplateArgument> * instantiation_arguments,
    Scope * declaration_scope)
{
  FunctionTemplateRegistrationIdentity identity;
  identity.decl = source_template;
  identity.key = instantiation_key;
  identity.arguments = instantiation_arguments;
  template_api::adopt_materialized_owner_template_identity(
      binding,
      identity,
      declaration_scope);
}

FunctionBinding * find_exact_function_binding(
    std::map<std::string, std::vector<FunctionBinding *> > & functions,
    const std::string & name,
    const TypePtr & type,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    RefQualifier ref_qualifier)
{
  const std::string canonical_name =
      semantic_lookup::canonical_function_lookup_name(name);
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      functions.find(canonical_name);
  if(found == functions.end()) {
    return nullptr;
  }

  for(std::size_t i = 0; i < found->second.size(); ++i) {
    if(types_equivalent_for_member_binding(found->second[i]->type, type) &&
       found->second[i]->ref_qualifier == ref_qualifier &&
       function_binding_matches_instantiation_identity(
           *found->second[i], source_template, instantiation_key)) {
      return found->second[i];
    }
  }
  return nullptr;
}

FunctionBinding * find_defined_function_binding(
    std::map<std::string, std::vector<FunctionBinding *> > & functions,
    const std::string & name,
    const TypePtr & type,
    FunctionTemplateDecl * source_template,
    const std::string & instantiation_key,
    RefQualifier ref_qualifier)
{
  const std::string canonical_name =
      semantic_lookup::canonical_function_lookup_name(name);
  std::map<std::string, std::vector<FunctionBinding *> >::iterator found =
      functions.find(canonical_name);
  if(found == functions.end()) {
    return nullptr;
  }

  FunctionBinding * fallback = nullptr;
  for(std::size_t i = 0; i < found->second.size(); ++i) {
    FunctionBinding * candidate = found->second[i];
    if(!types_equivalent_for_member_binding(candidate->type, type) ||
       candidate->ref_qualifier != ref_qualifier ||
       !function_binding_matches_instantiation_identity(
           *candidate, source_template, instantiation_key)) {
      continue;
    }
    if(!fallback) {
      fallback = candidate;
    }
    if(candidate->has_definition) {
      return candidate;
    }
  }
  return fallback;
}

void index_function_binding(FunctionRegistryState & state,
                            FunctionBinding * binding)
{
  if(!binding || binding->symbol.internal_symbol.empty()) {
    return;
  }
  state.functions_by_internal_symbol[binding->symbol.internal_symbol].push_back(
      binding);
}

void erase_indexed_function_binding(FunctionRegistryState & state,
                                    FunctionBinding * binding)
{
  if(!binding || binding->symbol.internal_symbol.empty()) {
    return;
  }
  std::unordered_map<std::string, std::vector<FunctionBinding *> >::iterator
      found =
          state.functions_by_internal_symbol.find(binding->symbol.internal_symbol);
  if(found == state.functions_by_internal_symbol.end()) {
    return;
  }
  erase_function_pointer(found->second, binding);
  if(found->second.empty()) {
    state.functions_by_internal_symbol.erase(found);
  }
}

void release_function_symbol_reservation(
    FunctionRegistryState & state,
    const FunctionBinding * binding,
    const FunctionBinding * retained_binding)
{
  if(!binding || binding->symbol.internal_symbol.empty()) {
    return;
  }
  if(retained_binding &&
     retained_binding->symbol.internal_symbol == binding->symbol.internal_symbol) {
    return;
  }
  if(!internal_symbol_has_other_function_owner(state,
                                               binding->symbol.internal_symbol,
                                               binding)) {
    state.used_internal_symbols.erase(binding->symbol.internal_symbol);
  }
}

void discard_function_binding(FunctionRegistryState & state,
                              FunctionBinding * binding)
{
  if(!binding) {
    return;
  }
  if(binding->output_emitted || binding->definition_output_emitted) {
    return;
  }
  erase_indexed_function_binding(state, binding);
  erase_function_pointer(state.instantiated_functions, binding);
  state.instantiated_function_set.erase(binding);
  erase_function_pointer(state.required_function_definitions, binding);
  state.required_function_definition_set.erase(binding);
  erase_function_pointer(state.late_required_class_methods, binding);
  state.late_required_class_method_set.erase(binding);
  erase_function_pointer(state.late_required_class_static_functions, binding);
  state.late_required_class_static_function_set.erase(binding);
  erase_function_pointer(state.synthetic_functions, binding);
  erase_function_pointer(state.deferred_constexpr_functions, binding);
  erase_function_binding_from_owner_lookups(binding);
  release_function_symbol_reservation(state, binding);
  for(std::vector<std::unique_ptr<FunctionBinding> >::iterator it =
          state.functions.begin();
      it != state.functions.end();
      ++it) {
    if(it->get() == binding) {
      state.live_functions.erase(binding);
      state.functions.erase(it);
      return;
    }
  }
}

void discard_class_function_bindings(FunctionRegistryState & state,
                                     ClassInfo & info)
{
  std::unordered_set<FunctionBinding *> stale;
  for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
          info.methods.begin();
      it != info.methods.end();
      ++it) {
    for(std::size_t i = 0; i < it->second.size(); ++i) {
      FunctionBinding * binding = it->second[i];
      if(function_binding_belongs_to_class(binding, info)) {
        stale.insert(binding);
      }
    }
  }
  if(info.member_scope) {
    for(std::map<std::string, std::vector<FunctionBinding *> >::const_iterator it =
            info.member_scope->function_sets.begin();
        it != info.member_scope->function_sets.end();
        ++it) {
      for(std::size_t i = 0; i < it->second.size(); ++i) {
        FunctionBinding * binding = it->second[i];
        if(function_binding_belongs_to_class(binding, info)) {
          stale.insert(binding);
        }
      }
    }
  }
  for(std::size_t i = 0; i < state.functions.size(); ++i) {
    FunctionBinding * binding = state.functions[i].get();
    if(function_binding_belongs_to_class(binding, info)) {
      stale.insert(binding);
    }
  }
  for(std::unordered_set<FunctionBinding *>::iterator it = stale.begin();
      it != stale.end();
      ++it) {
    discard_function_binding(state, *it);
  }
}

FunctionBinding * first_function_by_internal_symbol(
    const FunctionRegistryState & state,
    const std::string & internal_symbol)
{
  std::unordered_map<std::string, std::vector<FunctionBinding *> >::const_iterator
      found = state.functions_by_internal_symbol.find(internal_symbol);
  if(found == state.functions_by_internal_symbol.end() ||
     found->second.empty()) {
    return nullptr;
  }
  return found->second.front();
}

bool resolve_dump_callee_binding(
    const FunctionRegistryState & state,
    const FunctionRegistryCallbacks & callbacks,
    const CallSemNode & callee,
    FunctionBinding *& binding)
{
  binding = nullptr;
  if(callee.kind != CallSemKind::callee || !callee.semantic_type) {
    return false;
  }

  const symbol_linkage::SymbolIdentity & callee_symbol = callsem_symbol(callee);
  if(!callee_symbol.internal_symbol.empty()) {
    binding =
        first_function_by_internal_symbol(state, callee_symbol.internal_symbol);
    if(binding) {
      return true;
    }
  }

  for(std::size_t i = 0; i < state.functions.size(); ++i) {
    const bool legacy_match =
        state.functions[i]->name == callee.text &&
        type_equals(state.functions[i]->type, callee.semantic_type);
    if(legacy_match) {
      binding = state.functions[i].get();
      return true;
    }
  }
  return false;
}

}  // namespace callsemantic
