#include <functional>
#include <string>

int main()
{
  std::function<bool(const std::string &, const std::string &)> f;
  f = [&](const std::string & a, const std::string & b)
  {
    if(a.empty() || b.empty()) {
      return true;
    }
    return f(a.substr(1), b.substr(1));
  };
  return f("abc", "abc") ? 0 : 1;
}
