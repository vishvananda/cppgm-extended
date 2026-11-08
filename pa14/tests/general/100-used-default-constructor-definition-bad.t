struct Missing;
template<class T> struct Pointer {
  Pointer(T* value) { if (value) value->retain(); }
};
void used(Pointer<Missing> = nullptr);
int main() { used(); }
