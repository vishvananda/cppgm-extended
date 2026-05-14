#include <map>
#include <string>

int main() {
  std::map<std::string, int> m;
  std::string key = "x";
  m[std::move(key)] = 1;
  return m["x"] - 1;
}
