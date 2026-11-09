// N3485 5.3.4p11: default arguments complete an allocation call.
// expect exit 0
typedef decltype(sizeof(0)) size_t;
int evaluations, observed;
int default_extra() { ++evaluations; return 37; }
alignas(16) unsigned char storage[128];
struct Base {
  static void* operator new(size_t, int tag = 5, int extra = default_extra()) noexcept {
    observed = tag + extra;
    return storage;
  }
  static void* operator new[](size_t, int tag = 5, int extra = default_extra()) noexcept {
    observed = tag + extra;
    return storage;
  }
};
struct Item : Base { int value; Item() : value(9) {} };
int main() {
  Item* one = new (5) Item;
  if (evaluations != 1 || observed != 42 || one->value != 9) return 1;
  one->~Item();
  Item* two = new (6) Item[2];
  if (evaluations != 2 || observed != 43 || two[0].value != 9 || two[1].value != 9) return 2;
  two[1].~Item(); two[0].~Item();
  Item* explicit_extra = new (7, 8) Item;
  if (evaluations != 2 || observed != 15 || explicit_extra->value != 9) return 3;
  explicit_extra->~Item();
  Item* implicit = new Item;
  if (evaluations != 3 || observed != 42 || implicit->value != 9) return 4;
  implicit->~Item();
  return 0;
}
