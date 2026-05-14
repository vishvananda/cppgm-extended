#include "400-lazy-header-constexpr-static-assert.h"

struct S {
  void g()
  {
    struct G {};
    int x = sizeof(needs_complete<G>);
    (void)x;
  }
};

int main()
{
  S s;
  s.g();
}
