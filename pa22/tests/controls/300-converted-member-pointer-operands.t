// expect exit 0
struct Pad { long padding; };
struct Root { int value; int add(int n) const { return value + n; } };
struct Derived : Pad, Root {};
struct View { Derived* object; operator Derived*() const { return object; } };
struct Generic { Root* object; template<class T> operator T*() const { return object; } };
struct ConstGeneric { Root* object; template<class T> operator const T*() const { return object; } };
struct Member { operator int Root::*() const { return &Root::value; } };
struct Override {
    Derived* object;
    operator Derived*() const { return object; }
    int operator->*(int Root::*) const { return 71; }
};
int main() {
    Derived object;
    object.padding = 12345;
    object.value = 19;
    View view{&object};
    Generic generic{&object};
    ConstGeneric constant{&object};
    int Root::*data = &Root::value;
    int (Root::*method)(int) const = &Root::add;
    if ((view->*method)(7) != 26 || view->*data != 19) return 1;
    view->*data = 23;
    if ((generic->*method)(11) != 34 || constant->*data != 23) return 2;
    if (&object->*Member{} != 23) return 3;
    Override chosen{&object};
    if (chosen->*data != 71 || object.padding != 12345) return 4;
}
