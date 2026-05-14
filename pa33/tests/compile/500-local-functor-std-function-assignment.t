#include <functional>

struct S {
  std::function<void(int)> f;

  void g()
  {
    struct G {
      void operator()(int) const {}
    };
    f = G{};
  }
};

int main()
{
  S s;
  s.g();
}
