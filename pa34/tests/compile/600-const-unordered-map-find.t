#include <string>
#include <unordered_map>

int f(const std::unordered_map<std::string, int> & m,
      const std::string & k)
{
  return m.find(k) == m.end();
}
