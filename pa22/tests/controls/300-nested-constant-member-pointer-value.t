// expect exit 0
// N3485 focus: 3.2p2-p3, 9.8p1 and 5.1.2p11; constexpr scalar value versus object use.
// Regression family from GCC g++.dg/cpp0x/constexpr-__func__3.C (PR c++/105809).
struct Object { int value; int read() const { return value; } };
int main() {
  constexpr int Object::* data = &Object::value;
  constexpr int (Object::*function)() const = &Object::read;
  struct Local {
    int read(Object& object) { return object.*data + (object.*function)(); }
  };
  Object object = {17};
  return Local().read(object) != 34;
}
