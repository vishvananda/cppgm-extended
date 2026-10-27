// VALIDATION: compile-fail
// A dependent value is not a type merely because its binding is deferred.
template<class> struct Box {};
struct Base { static const int flag = 1; };
template<class T> struct Derived : T {
  using T::flag;
  void run() { Box<flag> box; }
};
int main() { Derived<Base> object; object.run(); }
