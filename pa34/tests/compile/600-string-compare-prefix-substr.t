#include <string>
#include <type_traits>
static_assert(std::string::npos == static_cast<std::string::size_type>(-1), "std::string::npos");
static_assert(std::is_same<std::string::value_type, char>::value, "value_type");
bool category_matches(const std::string & configured, const char * category)
{
  const std::size_t dot = configured.find(".*");
  if(dot != std::string::npos &&
     std::string(category).compare(0, dot, configured, 0, dot) == 0) { return true; }
  return false;
}
int main() { return category_matches("abc.*", "abcx") ? 0 : 1; }
