#include <sstream>
#include <string>

int main()
{
  std::istringstream in("abc");
  std::string out;
  std::getline(in, out);
  return out == "abc" ? 0 : 1;
}
