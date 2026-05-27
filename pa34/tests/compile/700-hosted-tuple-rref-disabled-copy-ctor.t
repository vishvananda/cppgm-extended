#include <string>
#include <tuple>

std::tuple<std::string&&>* p;

int main() {
  return p != 0;
}
