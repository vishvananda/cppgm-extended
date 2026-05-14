#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "callsemantic_internal.h"
#include "cpp_decl_model.h"

namespace semantic_cache {

typedef const std::string * InternedTextPtr;

struct ScopeTextKey
{
  std::size_t scope_key = 0;
  InternedTextPtr text = nullptr;

  bool operator==(const ScopeTextKey & rhs) const
  {
    return scope_key == rhs.scope_key && text == rhs.text;
  }
};

struct ScopeTextKeyHash
{
  std::size_t operator()(const ScopeTextKey & key) const;
};

struct ParsedTypeTextCacheKey
{
  std::size_t parse_scope_fingerprint = 0;
  std::size_t semantic_scope_key = 0;
  InternedTextPtr text = nullptr;
  bool reference_class_templates_only = false;

  bool operator==(const ParsedTypeTextCacheKey & rhs) const
  {
    return parse_scope_fingerprint == rhs.parse_scope_fingerprint &&
           semantic_scope_key == rhs.semantic_scope_key &&
           text == rhs.text &&
           reference_class_templates_only == rhs.reference_class_templates_only;
  }
};

struct ParsedTypeTextCacheKeyHash
{
  std::size_t operator()(const ParsedTypeTextCacheKey & key) const;
};

struct QualifiedTypeLookupKey
{
  std::size_t scope_key = 0;
  InternedTextPtr normalized_name = nullptr;
  bool reference_class_templates_only = false;
  bool allow_dependent_class_qualifiers = false;

  bool operator==(const QualifiedTypeLookupKey & rhs) const
  {
    return scope_key == rhs.scope_key &&
           normalized_name == rhs.normalized_name &&
           reference_class_templates_only == rhs.reference_class_templates_only &&
           allow_dependent_class_qualifiers == rhs.allow_dependent_class_qualifiers;
  }
};

struct QualifiedTypeLookupKeyHash
{
  std::size_t operator()(const QualifiedTypeLookupKey & key) const;
};

enum TextMentionCacheState
{
  TMCS_IN_PROGRESS,
  TMCS_FALSE,
  TMCS_TRUE
};

struct ParsedTypeTextCacheEntry
{
  bool parsed = false;
  cpp_decl::TypePtr type;
};

struct DependentTypeResolutionCacheEntry
{
  enum Status { DTS_IN_PROGRESS, DTS_UNRESOLVED, DTS_RESOLVED } status = DTS_UNRESOLVED;
  cpp_decl::TypePtr resolved;
};

struct DependentTypeResolutionCacheKey
{
  std::size_t scope_key = 0;
  InternedTextPtr type_key = nullptr;

  bool operator==(const DependentTypeResolutionCacheKey & rhs) const
  {
    return scope_key == rhs.scope_key && type_key == rhs.type_key;
  }
};

struct DependentTypeResolutionCacheKeyHash
{
  std::size_t operator()(const DependentTypeResolutionCacheKey & key) const;
};

struct DependentTypeResolutionCacheKeyLess
{
  bool operator()(const DependentTypeResolutionCacheKey & lhs,
                  const DependentTypeResolutionCacheKey & rhs) const
  {
    if(lhs.scope_key != rhs.scope_key) {
      return lhs.scope_key < rhs.scope_key;
    }
    return lhs.type_key < rhs.type_key;
  }
};

struct SemanticCache
{
  std::unordered_map<std::size_t, semantic_model::Scope *> captured_local_scope_cache;
  mutable std::unordered_set<std::string> interned_text_pool;
  mutable std::unordered_map<InternedTextPtr, callsemantic_internal::IdentifierTokenSet>
      identifier_token_cache;
  mutable std::unordered_map<ScopeTextKey,
                             TextMentionCacheState,
                             ScopeTextKeyHash>
      template_placeholder_mentions_cache;
  mutable std::unordered_map<ScopeTextKey, bool, ScopeTextKeyHash>
      non_namespace_binding_mentions_cache;
  mutable std::unordered_map<ScopeTextKey, bool, ScopeTextKeyHash>
      dependent_non_namespace_binding_mentions_cache;
  mutable std::unordered_map<QualifiedTypeLookupKey,
                             cpp_decl::TypePtr,
                             QualifiedTypeLookupKeyHash>
      qualified_type_lookup_cache;
  mutable std::unordered_map<ParsedTypeTextCacheKey,
                             ParsedTypeTextCacheEntry,
                             ParsedTypeTextCacheKeyHash>
      parsed_type_text_cache;
  // Keep dependent type resolution keyed by stable semantic identity rather
  // than TypePtr graph structure. Reuse interned semantic text so this cache
  // follows the same pointer-keyed lifetime model as the other semantic caches.
  mutable std::map<DependentTypeResolutionCacheKey,
                   DependentTypeResolutionCacheEntry,
                   DependentTypeResolutionCacheKeyLess>
      dependent_type_resolution_cache;

  void clear_scope_capture_cache();
  void dump(std::ostream & out) const;
  InternedTextPtr intern_text(const std::string & text) const;
};

bool stats_enabled();

}  // namespace semantic_cache
