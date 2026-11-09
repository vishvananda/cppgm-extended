// N3485 14.7.1p12: instantiate only allocation defaults that are used.
// expect exit 0
typedef decltype(sizeof(0)) size_t;
alignas(16) unsigned char storage[64];
int observed;
struct Good { static int extra() { return 41; } };
struct Missing {};
template<class T> struct Item {
  static void* operator new(size_t, int extra = T::extra()) noexcept {
    observed = extra; return storage;
  }
  int value;
  Item() : value(9) {}
};
int main() {
  Item<Good>* one = new Item<Good>;
  if (observed != 41 || one->value != 9) return 1;
  one->~Item<Good>();
  Item<Missing>* two = new (17) Item<Missing>;
  if (observed != 17 || two->value != 9) return 2;
  two->~Item<Missing>();
  return 0;
}
