#include <algorithm>
#include <string>
#include <vector>

std::string f1()
{
  struct Local { std::string s; int x; };
  std::vector<Local> v(2);
  std::sort(v.begin(), v.end(), [](const Local& a, const Local& b) { return a.s < b.s; });
  return std::string();
}

std::string f2()
{
  struct Local { std::string s; int x; };
  std::vector<Local> v(2);
  std::sort(v.begin(), v.end(), [](const Local& a, const Local& b) { return a.s < b.s; });
  return std::string();
}

int main()
{
  return 0;
}
