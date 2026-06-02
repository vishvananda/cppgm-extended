#include <string>
#include <unordered_set>
#include <type_traits>
static_assert(std::is_same<std::unordered_set<std::string>::value_type, std::string>::value, "unordered_set value_type");
void add_names(std::unordered_set<std::string> & target, const std::unordered_set<std::string> & source)
{ target.insert(source.begin(), source.end()); }
int main() { std::unordered_set<std::string> target; std::unordered_set<std::string> source; add_names(target, source); return 0; }
