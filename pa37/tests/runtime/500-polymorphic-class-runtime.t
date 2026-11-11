#include <typeinfo>
struct Root { int member; Root(int x) : member(x) {} virtual int value() { return member; } };
struct Other { int other; Other(int x) : other(x) {} virtual int second() { return other; } };
struct Derived : Root, Other {
 int extra;
 Derived() : Root(11), Other(7), extra(3) {}
 int value() override { return member + extra; }
 int second() override { return other + extra; }
};
extern "C" int visit(Root*);
extern "C" int visit_other(Other*);
extern "C" Other* cross(Root*);
extern "C" Root* instance();
extern "C" int inspect(Root*, Other*);
extern "C" int observe() {
 Derived object;
 Root* root = &object;
 Other* other = &object;
 if (visit(root) != 14 || visit_other(other) != 10) return 1;
 if (cross(root) != other || dynamic_cast<Derived*>(other) != &object) return 2;
 if (dynamic_cast<void*>(other) != static_cast<void*>(&object)) return 3;
 if (typeid(*other) != typeid(Derived) || typeid(*root) != typeid(Derived)) return 4;
 int (Root::*first)() = &Root::value;
 int (Other::*second)() = &Other::second;
 if ((object.*first)() != 14 || (object.*second)() != 10) return 5;
 if (instance() != instance() || visit(instance()) != 14 || !inspect(root, other)) return 6;
 return 17;
}
