#include <string>
#include <utility>
#include <type_traits>
static_assert(std::is_same<decltype(std::declval<std::string>() + std::declval<char>()), std::string>::value,
              "std::string + char -> std::string");
int main()
{
  std::string delimiter = "";
  char quote = '"';
  std::string terminator = std::string(")") + delimiter + quote;
  return terminator == ")\"" ? 0 : 1;
}
