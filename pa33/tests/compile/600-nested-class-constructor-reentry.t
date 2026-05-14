#include <vector>

struct Outer {
  struct PPToken {
    int value;
  };

  struct Macro {
    Macro() {}
    std::vector<PPToken> tokens;
  };

  std::vector<PPToken> cur_arg;
};

int main() {
  Outer o;
  return (int)o.cur_arg.size();
}
