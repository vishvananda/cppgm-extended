#include "semantic_cache.h"

#include <cstdlib>
#include <functional>
#include <ostream>

namespace semantic_cache {

namespace {

template <typename T>
void hash_combine(std::size_t & seed, const T & value)
{
  seed ^= std::hash<T>()(value) + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

}  // namespace

std::size_t ScopeTextKeyHash::operator()(const ScopeTextKey & key) const
{
  std::size_t seed = 0;
  hash_combine(seed, key.scope_key);
  hash_combine(seed, key.text);
  return seed;
}

std::size_t QualifiedTypeLookupKeyHash::operator()(const QualifiedTypeLookupKey & key) const
{
  std::size_t seed = 0;
  hash_combine(seed, key.scope_key);
  hash_combine(seed, key.normalized_name);
  hash_combine(seed, key.reference_class_templates_only);
  hash_combine(seed, key.allow_dependent_class_qualifiers);
  return seed;
}

void SemanticCache::dump(std::ostream & out) const
{
  out << "semantic-cache"
      << " capture.scope=" << captured_local_scope_cache.size()
      << " text.interned_pool=" << interned_text_pool.size()
      << " text.identifier_tokens=" << identifier_token_cache.size()
      << " mention.template_placeholder=" << template_placeholder_mentions_cache.size()
      << " mention.non_namespace=" << non_namespace_binding_mentions_cache.size()
      << " mention.dependent_non_namespace="
      << dependent_non_namespace_binding_mentions_cache.size()
      << " lookup.qualified_type=" << qualified_type_lookup_cache.size()
      << '\n';
}

InternedTextPtr SemanticCache::intern_text(const std::string & text) const
{
  return &*interned_text_pool.insert(text).first;
}

bool stats_enabled()
{
  static const bool enabled = []()
  {
    const char * value = std::getenv("CPPGM_SEMANTIC_CACHE_STATS");
    return value && *value && std::string(value) != "0";
  }();
  return enabled;
}

}  // namespace semantic_cache
