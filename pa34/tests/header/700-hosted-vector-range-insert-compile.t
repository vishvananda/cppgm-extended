#include <vector>
#include <type_traits>
static_assert(std::is_same<std::vector<int>::value_type, int>::value, "<vector> value_type");
int main() { std::vector<int> a; std::vector<int> b; a.insert(a.begin(), b.begin(), b.end()); return 0; }
