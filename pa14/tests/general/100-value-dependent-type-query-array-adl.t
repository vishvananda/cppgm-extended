// A sizeof/alignof query has a fixed result type but can have a dependent
// value. The local array type keeps that dependence for operator ADL.
struct Stream {
  int selected;
  Stream& operator<<(const void*) { selected = 1; return *this; }
};
template<class T> void bytes(Stream& out) {
  char text[sizeof(T)] = {};
  out << text;
}
template<class T> void alignment(Stream& out) {
  char text[alignof(T)] = {};
  out << text;
}
template<class T> void alias(Stream& out) {
  typedef char Text[sizeof(T)];
  Text text{};
  out << text;
}
template<class T> void fixed(Stream& out) {
  char text[sizeof(int)] = {};
  out << text;
}
Stream& operator<<(Stream& out, const char*) { out.selected = 2; return out; }
int main() {
  Stream out{};
  bytes<int>(out);
  if (out.selected != 2) return 1;
  alignment<int>(out);
  if (out.selected != 2) return 2;
  alias<int>(out);
  if (out.selected != 2) return 3;
  fixed<int>(out);
  return out.selected != 1;
}
