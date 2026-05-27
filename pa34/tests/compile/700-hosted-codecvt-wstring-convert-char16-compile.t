#include <codecvt>
#include <locale>

int main()
{
  std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> test;
  (void)test;
  return 0;
}
