namespace fallback { template<class T> void swap(T&, T&) noexcept(false); }
namespace impl {
using namespace fallback;
template<class T> void invoke(T& a, T& b) noexcept(noexcept(swap(a,b))) { swap(a,b); }
}
namespace client {
struct Quiet { static Quiet& instance() noexcept; friend void swap(Quiet&,Quiet&) noexcept {} };
struct Loud { static Loud& instance() noexcept; friend void swap(Loud&,Loud&) {} };
}
static_assert(noexcept(impl::invoke(client::Quiet::instance(),client::Quiet::instance())), "quiet call");
static_assert(!noexcept(impl::invoke(client::Loud::instance(),client::Loud::instance())), "loud call");
int main() { return 0; }
