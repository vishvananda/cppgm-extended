namespace std { class type_info; }
template<class T> struct Identity { typedef T type; };
struct Base { virtual ~Base() {} };
struct Derived : Base {};
Derived object;
struct Store {
  static Base& base;
  static int integer;
  static int calls;
  static Base& get() { ++calls; return base; }
};
Base& Store::base=object;
int Store::calls;
typedef Identity<Store>::type Alias;
int main() { return &typeid(Alias::base)!=&typeid(Derived) || &typeid(Alias::integer)!=&typeid(int) || &typeid(Alias::get())!=&typeid(Derived) || Alias::calls!=1; }
