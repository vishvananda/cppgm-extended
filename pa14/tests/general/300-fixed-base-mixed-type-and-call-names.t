struct State {
  struct item {int value;};
  item get() const {return {7};}
};
template<class T> struct Wrapper : T {};
struct Derived : Wrapper<State> {
  int read() const {item local = get(); return local.value;}
};
int main() {Derived d; return d.read() == 7 ? 0 : 1;}
