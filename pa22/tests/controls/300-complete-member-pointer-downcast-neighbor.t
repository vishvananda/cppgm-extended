// expect exit 0
// N3485 5.2.9p12: a complete derived class permits the inverse member-pointer conversion.
struct Base { int a; }; struct Derived:Base { int b; }; constexpr int Base::* pointer=(int Base::*)&Derived::b; static_assert(pointer!=nullptr,"complete conversion"); int main() { Derived object; object.b=7; return object.*pointer!=7; }
