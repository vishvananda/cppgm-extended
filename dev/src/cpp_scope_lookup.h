#pragma once

#include <cstddef>
#include <set>
#include <string>

#include "cpp_decl_model.h"

namespace cpp_scope_lookup {

template<typename Result>
bool lookup_results_same(const Result & lhs, const Result & rhs)
{
  return lhs == rhs;
}

inline bool lookup_results_same(const cpp_decl::TypePtr & lhs,
                                const cpp_decl::TypePtr & rhs)
{
  return cpp_decl::type_equals(lhs, rhs);
}

template<typename ScopeT>
auto direct_declaration_hides_using_directives(const ScopeT & scope, int)
    -> decltype(scope.namespace_scope, bool())
{
  return !scope.namespace_scope;
}

template<typename ScopeT>
bool direct_declaration_hides_using_directives(const ScopeT &, long)
{
  return false;
}

template<typename Result,
         typename ScopeT,
         typename DirectLookup,
         typename HasResult>
void collect_lookup_from_using_directives(ScopeT & scope,
                                          const std::string & name,
                                          std::set<const ScopeT *> & visited,
                                          const DirectLookup & direct_lookup,
                                          const HasResult & has_result,
                                          bool & found,
                                          Result & value,
                                          bool & ambiguous)
{
  if(!visited.insert(&scope).second) {
    return;
  }

  for(size_t i = 0; i < scope.using_directives.size(); ++i) {
    ScopeT * imported = scope.using_directives[i];
    Result direct = direct_lookup(*imported, name);
    if(has_result(direct)) {
      if(!found) {
        found = true;
        value = direct;
      } else if(!lookup_results_same(value, direct)) {
        ambiguous = true;
        return;
      }
    }

    collect_lookup_from_using_directives<Result>(
        *imported,
        name,
        visited,
        direct_lookup,
        has_result,
        found,
        value,
        ambiguous);
    if(ambiguous) {
      return;
    }
  }
}

template<typename Result,
         typename ScopeT,
         typename DirectLookup,
         typename HasResult>
Result lookup_unqualified(ScopeT & scope,
                          const std::string & name,
                          const DirectLookup & direct_lookup,
                          const HasResult & has_result,
                          bool * ambiguous_result = nullptr)
{
  for(ScopeT * current = &scope; current; current = current->parent) {
    bool found_at_level = false;
    bool ambiguous_at_level = false;
    Result result_at_level;
    Result direct = direct_lookup(*current, name);
    if(has_result(direct)) {
      if(direct_declaration_hides_using_directives(*current, 0)) {
        return direct;
      }
      found_at_level = true;
      result_at_level = direct;
    }

    std::set<const ScopeT *> visited;
    collect_lookup_from_using_directives<Result>(
        *current,
        name,
        visited,
        direct_lookup,
        has_result,
        found_at_level,
        result_at_level,
        ambiguous_at_level);
    if(ambiguous_at_level) {
      if(ambiguous_result) {
        *ambiguous_result = true;
      }
      return Result();
    }
    if(found_at_level) {
      return result_at_level;
    }
  }

  return Result();
}

template<typename Result,
         typename ScopeT,
         typename ResolveUnqualifiedQualifier,
         typename ResolveDirectQualifier,
         typename FinalLookup>
Result lookup_qualified(ScopeT & root,
                        const cpp_decl::QualifiedName & qualified,
                        const ResolveUnqualifiedQualifier & resolve_unqualified_qualifier,
                        const ResolveDirectQualifier & resolve_direct_qualifier,
                        const FinalLookup & final_lookup)
{
  if(qualified.rooted && qualified.qualifiers.empty()) {
    return final_lookup(root, qualified.name);
  }

  if(!qualified.rooted && qualified.qualifiers.empty()) {
    return Result();
  }

  ScopeT * current = nullptr;
  size_t index = 0;
  if(qualified.rooted) {
    current = &root;
  } else {
    current = resolve_unqualified_qualifier(qualified.qualifiers[0]);
    if(!current) {
      return Result();
    }
    index = 1;
  }

  for(; index < qualified.qualifiers.size(); ++index) {
    current = resolve_direct_qualifier(*current, qualified.qualifiers[index]);
    if(!current) {
      return Result();
    }
  }

  return final_lookup(*current, qualified.name);
}

}  // namespace cpp_scope_lookup
