namespace fallback { template<class... T> void operation(T&...) noexcept(false); }
namespace dispatch {
using namespace fallback;
template<class... T> void invoke(T&... args) noexcept(noexcept(operation(args...))) {}
}
namespace client {
struct Quiet {
  static Quiet& instance() noexcept;
  friend void operation(Quiet&,Quiet&) noexcept {}
};
struct Loud {
  static Loud& instance() noexcept;
  friend void operation(Loud&,Loud&) {}
};
}
static_assert(noexcept(dispatch::invoke(client::Quiet::instance(),client::Quiet::instance())), "expanded ADL");
static_assert(!noexcept(dispatch::invoke(client::Loud::instance(),client::Loud::instance())), "throwing ADL");
static_assert(!noexcept(dispatch::invoke()), "empty pack uses fallback");
int main() { return 0; }
