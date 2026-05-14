#include <string>

struct E {};

extern void sink(const char *, const std::string &);

void probe() {
  try {
    throw E();
  } catch(E &) {
    std::string s("a");
    sink("x", s);
  }

  try {
    throw E();
  } catch(E &) {
    std::string s("a");
    sink("x", s);
  }
}
