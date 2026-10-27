namespace fixed { struct Value {}; Value& instance() noexcept; }
namespace fallback { template<class T> void swap(T&,T&) noexcept(false); }
namespace impl {
using namespace fallback;
template<class... T> void invoke(fixed::Value& a, T...)
  noexcept(noexcept(swap(a,a))) {}
template<class... T> void after(T... args, fixed::Value& a)
  noexcept(noexcept(swap(a,a))) {}
}
namespace fixed { void swap(Value&,Value&) noexcept; }
static_assert(!noexcept(impl::invoke(fixed::instance(), 1, 2)), "fixed before pack");
static_assert(!noexcept(impl::invoke(fixed::instance())), "empty pack keeps fixed prefix");
static_assert(!noexcept(impl::invoke(fixed::instance(), 1)), "one element keeps fixed prefix");
static_assert(!noexcept(impl::after<int,int>(1,2,fixed::instance())), "fixed suffix after pack");
int main() { return 0; }
