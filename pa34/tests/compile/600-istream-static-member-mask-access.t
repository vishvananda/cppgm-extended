#include <sstream>
#include <string>

int main() {
  std::istringstream in("abc");
  std::string out;
  in >> out;
  return out == "abc" ? 0 : 1;
}
