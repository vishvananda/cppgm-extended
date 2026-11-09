// A used allocation default is instantiated, even when allocation is noexcept.
// expect reject
typedef decltype(sizeof(0)) size_t;
struct Missing {};
template<class T> struct Item {
  static void* operator new(size_t, int extra = T::extra()) noexcept;
};
int main() { new Item<Missing>; }
