#include <vector>

struct Incomplete;

struct Context {
  virtual void f(const std::vector<Incomplete>& args) = 0;
  virtual const std::vector<Incomplete>& g() const = 0;
};

int main() { return 0; }
