#include <vector>
#include <type_traits>
static_assert(std::is_same<std::vector<int*>::value_type, int*>::value, "<vector> value_type");
namespace real { struct Function { int value; }; }
namespace outer {
namespace alias = real;
namespace { std::vector<const alias::Function *> functions; }
int check() { return functions.empty() ? 0 : 1; }
}
int main() { return outer::check(); }
