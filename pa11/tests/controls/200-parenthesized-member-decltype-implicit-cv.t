// expect exit 0
int choose(int&) { return 1; }
int choose(const int&) { return 2; }
struct Base { int value; };
struct Derived : Base {
 int test() const { decltype((Base::value)) ref = value; return choose(ref); }
};
int main() { Derived d; d.value = 7; return d.test() == 2 ? 0 : 1; }
