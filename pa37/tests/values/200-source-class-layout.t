struct Root { virtual int value(); int member; };
struct EmptyRoot { virtual int value(); };
struct Derived : Root { int other; };
struct Other { virtual int other(); int member; };
struct Multiple : Root, Other { int last; };
struct Wide { virtual int value(); long long member; };
struct Container { Root* pointer; Root member; };
static_assert(sizeof(Root) == 8 && alignof(Root) == 4, "target vptr layout");
static_assert(sizeof(EmptyRoot) == 4, "target nearly empty class");
static_assert(sizeof(Derived) == 12, "primary base layout");
static_assert(sizeof(Multiple) == 20, "multiple dynamic bases");
static_assert(sizeof(Wide) == 16 && alignof(Wide) == 8, "wide field alignment");
static_assert(sizeof(Container) == 12, "nested dynamic member layout");
static_assert(sizeof(int Root::*) == 4, "target data member pointer");
static_assert(sizeof(int (Root::*)()) == 8, "target function member pointer");
extern "C" int observe() { return 17; }
