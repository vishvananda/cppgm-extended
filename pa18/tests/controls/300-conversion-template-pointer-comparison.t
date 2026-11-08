// expect exit 0
struct View {
  unsigned char* pointer;
  template<class T> operator T*() const { return pointer; }
};
struct ExplicitView {
  unsigned char* pointer;
  template<class T> explicit operator T*() const { return pointer; }
};
bool equal(View value, const unsigned char* pointer) { return value == pointer; }
bool reverse(const unsigned char* pointer, View value) { return pointer == value; }
int main() {
  unsigned char bytes[3] = {1,2,3};
  View value = {bytes};
  return !equal(value, bytes) || equal(value, bytes + 1) || !reverse(bytes, value) ||
    !(value < static_cast<const unsigned char*>(bytes + 2));
}
