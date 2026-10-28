// A fixed-type integral parameter makes an array's bound value-dependent.
// Constants retain that provenance. Array parameters lose the bound when
// adjusted to pointers, while references retain their dependent array type.
struct Stream {
  int selected;
  Stream& operator<<(const void*) { selected = 1; return *this; }
};
template<unsigned N> void bound(Stream& out) {
  char text[N] = {};
  out << text;
}
template<unsigned N> void constant(Stream& out) {
  const unsigned count = N;
  char text[count] = {};
  out << text;
}
template<unsigned N> void alias(Stream& out) {
  typedef char Text[N - N + 3];
  Text text{};
  out << text;
}
template<unsigned N> void parameter(Stream& out, char text[N]) { out << text; }
template<unsigned N> void reference(Stream& out, char (&text)[N]) { out << text; }
Stream& operator<<(Stream& out, const char*) { out.selected = 2; return out; }
int main() {
  Stream out{};
  bound<3>(out);
  if (out.selected != 2) return 1;
  constant<3>(out);
  if (out.selected != 2) return 2;
  alias<3>(out);
  if (out.selected != 2) return 3;
  char text[3] = {};
  parameter<3>(out, text);
  if (out.selected != 1) return 4;
  reference<3>(out, text);
  return out.selected != 2;
}
