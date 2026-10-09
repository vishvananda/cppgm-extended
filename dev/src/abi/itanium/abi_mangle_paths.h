#pragma once

// Interned strings and qualified-name paths of the numeric ABI mangling
// graph.  A path is a parent-linked chain of interned components, so equal
// qualified names share one path identity.

#include "abi/itanium/abi_mangle_facts.h"
#include "abi/itanium/abi_mangle_stats.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace abi_mangle {
namespace detail {

class StringPool
{
public:
  std::size_t intern(const std::string & value);
  const std::string & get(std::size_t id) const;

private:
  std::vector<std::string> values_;
  std::unordered_map<std::string, std::size_t> indexes_;
};

struct PathNode
{
  std::size_t parent = ABI_NO_RESOLVED_REFERENCE;
  std::size_t name = ABI_NO_RESOLVED_REFERENCE;

  PathNode() {}
  PathNode(std::size_t parent_value, std::size_t name_value)
    : parent(parent_value), name(name_value) {}

  bool operator==(const PathNode & other) const
  {
    return parent == other.parent && name == other.name;
  }
};

struct PathHash
{
  std::size_t operator()(const PathNode & path) const;
};

class PathPool
{
public:
  PathPool(StringPool & strings, AbiMangleStats * stats)
    : strings_(strings), stats_(stats) {}

  std::size_t intern(const std::string & qualified,
                     std::size_t * origin_components = nullptr);
  std::size_t intern(std::size_t parent, std::size_t name);
  std::size_t intern(const std::vector<std::size_t> & components);
  const PathNode & get(std::size_t id) const;
  std::vector<std::size_t> components(std::size_t id) const;
  std::vector<std::size_t> prefixes(std::size_t id) const;

private:
  StringPool & strings_;
  AbiMangleStats * stats_;
  std::vector<PathNode> paths_;
  std::unordered_map<PathNode, std::size_t, PathHash> indexes_;
};

}  // namespace detail
}  // namespace abi_mangle
