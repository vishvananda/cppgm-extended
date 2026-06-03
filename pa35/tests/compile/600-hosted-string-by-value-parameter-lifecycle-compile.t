#include <string>
#include <utility>
#include <type_traits>
static_assert(std::is_move_constructible<std::string>::value, "std::string move-constructible");
struct Holder { Holder(std::string s) : value(std::move(s)) {} std::string value; };
int main() { Holder holder((std::string())); (void)holder; return 0; }
