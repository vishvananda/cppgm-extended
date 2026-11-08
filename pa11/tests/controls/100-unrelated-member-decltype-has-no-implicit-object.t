// expect exit 0
// N3485 9.3.1p3: an unrelated member in an unevaluated operand does not use this.
int choose(int&) { return 1; }
int choose(const int&) { return 2; }
struct Other { int value; };
struct Item { int test() const { int local = 0; decltype((Other::value)) ref = local; return choose(ref); } };
int main() { Item i; return i.test() == 1 ? 0 : 1; }
