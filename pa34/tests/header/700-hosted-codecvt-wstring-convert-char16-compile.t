#include <codecvt>
#include <locale>
#include <type_traits>
static_assert(std::is_same<std::codecvt_utf8<char16_t>::intern_type, char16_t>::value, "codecvt_utf8 intern_type");
int main() { std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> test; (void)test; return 0; }
