#include "abi/itanium/abi_mangle_paths.h"

#include "abi/itanium/abi_mangle_errors.h"
#include "abi/itanium/abi_mangle_hash.h"

namespace abi_mangle {
namespace detail {

std::size_t StringPool::intern(const std::string & value)
{
  const auto found = indexes_.find(value);
  if(found != indexes_.end()) return found->second;
  const std::size_t id = values_.size();
  values_.push_back(value);
  indexes_.insert(std::make_pair(value, id));
  return id;
}

const std::string & StringPool::get(std::size_t id) const
{
  if(id >= values_.size()) ThrowAbiInternal("invalid interned ABI string id");
  return values_[id];
}

std::size_t PathHash::operator()(const PathNode & path) const
{
  return mix_hash(path.parent, path.name);
}

std::size_t PathPool::intern(const std::string & qualified,
                             std::size_t * origin_components)
{
  std::size_t begin = qualified.compare(0, 2, "::") == 0 ? 2 : 0;
  std::size_t path = ABI_NO_RESOLVED_REFERENCE;
  while(begin < qualified.size()) {
    const std::size_t separator = qualified.find("::", begin);
    const std::string component = qualified.substr(begin, separator - begin);
    if(component.empty()) {
      ThrowAbiInternal("empty component in ABI name '" + qualified + "'");
    }
    if(stats_) ++stats_->path_components;
    if(origin_components) ++*origin_components;
    path = intern(path, strings_.intern(component));
    if(separator == std::string::npos) break;
    begin = separator + 2;
  }
  if(path == ABI_NO_RESOLVED_REFERENCE) ThrowAbiInternal("empty ABI qualified name");
  return path;
}

std::size_t PathPool::intern(std::size_t parent, std::size_t name)
{
  const PathNode key{parent, name};
  const auto found = indexes_.find(key);
  if(found != indexes_.end()) return found->second;
  const std::size_t id = paths_.size();
  paths_.push_back(key);
  indexes_.insert(std::make_pair(key, id));
  return id;
}

std::size_t PathPool::intern(const std::vector<std::size_t> & components)
{
  if(components.empty()) ThrowAbiInternal("empty ABI component path");
  std::size_t path = ABI_NO_RESOLVED_REFERENCE;
  for(std::size_t component : components) path = intern(path, component);
  return path;
}

const PathNode & PathPool::get(std::size_t id) const
{
  if(id >= paths_.size()) ThrowAbiInternal("invalid ABI path id");
  return paths_[id];
}

std::vector<std::size_t> PathPool::components(std::size_t id) const
{
  std::vector<std::size_t> reverse;
  while(id != ABI_NO_RESOLVED_REFERENCE) {
    reverse.push_back(paths_[id].name);
    id = paths_[id].parent;
  }
  return std::vector<std::size_t>(reverse.rbegin(), reverse.rend());
}

std::vector<std::size_t> PathPool::prefixes(std::size_t id) const
{
  std::vector<std::size_t> reverse;
  while(id != ABI_NO_RESOLVED_REFERENCE) {
    reverse.push_back(id);
    id = paths_[id].parent;
  }
  return std::vector<std::size_t>(reverse.rbegin(), reverse.rend());
}

}  // namespace detail
}  // namespace abi_mangle
