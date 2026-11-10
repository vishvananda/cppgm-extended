// A zero-capacity container may retain the one-past address of its own
// frame storage. Callee-saved registers must shift that boundary along with
// the rest of the object when the frame pointer is omitted.
volatile unsigned observed;
struct Vector {
  void *begin;
  unsigned size, capacity;
  Vector():begin(reinterpret_cast<char*>(this)+sizeof(Vector)),size(0),capacity(0){}
  __attribute__((noinline)) unsigned check() const {
    return begin == reinterpret_cast<const char*>(this)+sizeof(Vector) ? 0 : 1;
  }
  __attribute__((noinline)) ~Vector() {observed += size;}
};
struct Holder {
  Vector first, second, third, last;
  __attribute__((noinline)) unsigned check() const {
    return first.check() | second.check() | third.check() | last.check();
  }
};
__attribute__((noinline)) void touch() {observed += 1;}
__attribute__((noinline)) unsigned run(unsigned x) {Holder h; unsigned result=h.check(); touch(); return result+x;}
int main() {return run(observed);}
