namespace fallback { int probe(...) noexcept(false) { return 9; } }
namespace impl {
using namespace fallback;
template<class T> struct Wrapper {
  static int test(T value) noexcept(noexcept(probe(value))) { return probe(value); }
};
}
namespace client {
struct Quiet { friend int probe(Quiet) noexcept { return 7; } };
struct Loud { friend int probe(Loud) { return 8; } };
}
static_assert(noexcept(impl::Wrapper<client::Quiet>::test(client::Quiet{})), "dependent lookup");
static_assert(!noexcept(impl::Wrapper<client::Loud>::test(client::Loud{})), "throwing lookup");
int main() { return impl::Wrapper<client::Quiet>::test(client::Quiet{}) != 7 || impl::Wrapper<client::Loud>::test(client::Loud{}) != 8; }
