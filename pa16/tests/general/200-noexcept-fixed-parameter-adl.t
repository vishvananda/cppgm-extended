namespace fixed_client { struct Value {}; Value& instance() noexcept; }
namespace fallback { template<class T> void swap(T&,T&) noexcept(false); }
namespace impl {
using namespace fallback;
template<class T> void invoke(fixed_client::Value& a, fixed_client::Value& b)
  noexcept(noexcept(swap(a,b))) { swap(a,b); }
}
namespace fixed_client { void swap(Value&,Value&) noexcept; }
static_assert(!noexcept(impl::invoke<int>(fixed_client::instance(),fixed_client::instance())),
              "fixed parameter lookup remains bound at template definition");
int main() { return 0; }
