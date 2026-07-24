#include "text_intern.h"

#include <string>
#include <unordered_set>

namespace text_intern {

namespace {

std::unordered_set<std::string> & atom_pool()
{
  static std::unordered_set<std::string> pool;
  static bool reserved = false;
  if(!reserved) {
    pool.reserve(16384);
    reserved = true;
  }
  return pool;
}

}  // namespace

Atom intern(const std::string & text)
{
  std::unordered_set<std::string> & pool = atom_pool();
  return &*pool.insert(text).first;
}

Atom intern(std::string && text)
{
  std::unordered_set<std::string> & pool = atom_pool();
  return &*pool.insert(std::move(text)).first;
}

Atom intern(const char * data, std::size_t length)
{
  std::unordered_set<std::string> & pool = atom_pool();
  return &*pool.insert(std::string(data, length)).first;
}

Atom find(const std::string & text)
{
  std::unordered_set<std::string> & pool = atom_pool();
  std::unordered_set<std::string>::const_iterator found = pool.find(text);
  return found == pool.end() ? nullptr : &*found;
}

std::size_t atom_count()
{
  return atom_pool().size();
}

std::size_t storage_bytes()
{
  std::size_t bytes = 0;
  const std::unordered_set<std::string> & pool = atom_pool();
  for(std::unordered_set<std::string>::const_iterator it = pool.begin();
      it != pool.end();
      ++it) {
    bytes += it->capacity();
  }
  return bytes;
}

}  // namespace text_intern
