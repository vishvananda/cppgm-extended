struct Missing;
template<class T> struct Pointer {
  Pointer(T* value) { if (value) value->retain(); }
};
void unused(Pointer<Missing> = nullptr);
int calls;
struct Ready { void retain() { ++calls; } };
void used(Pointer<Ready> = nullptr) { ++calls; }
int main() {
  used();
  Ready value;
  used(&value);
  return calls == 3 ? 0 : 1;
}
