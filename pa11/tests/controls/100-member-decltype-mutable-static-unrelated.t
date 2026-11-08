// expect exit 0
int choose(int&) { return 1; }
int choose(const int&) { return 2; }
struct Other { int value; };
struct Item {
 mutable int value; static int shared;
 int test() const {
  decltype((value)) a = value;
  decltype((shared)) b = shared;
  decltype(value) d = 0;
  return choose(a) + choose(b) + choose(d);
 }
};
int Item::shared = 0;
int main() { const Item i = { 0 }; return i.test() == 3 ? 0 : 1; }
