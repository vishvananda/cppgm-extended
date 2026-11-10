struct Fields { unsigned tag:7; unsigned rest:25; };
__attribute__((noinline)) long chosen(const Fields* p, long yes, long no) {
  return p->tag ? yes : no;
}
__attribute__((noinline)) long equal_zero(const Fields* p, long yes, long no) {
  return p->tag == 0 ? yes : no;
}
__attribute__((noinline)) long before_store(Fields* p, long yes, long no) {
  unsigned old = p->tag;
  p->tag = 0;
  return old ? yes : no;
}
__attribute__((noinline)) long observed(const volatile Fields* p, long yes, long no) {
  return p->tag ? yes : no;
}
int main() {
  Fields fields;
  for (int i = -256; i != 256; ++i) {
    fields.tag = static_cast<unsigned>(i);
    fields.rest = 12345;
    long yes = i + 11;
    long no = i - 29;
    long expected = (static_cast<unsigned>(i) & 127) ? yes : no;
    long opposite = (static_cast<unsigned>(i) & 127) ? no : yes;
    if (chosen(&fields, yes, no) != expected) return 1;
    if (equal_zero(&fields, yes, no) != opposite) return 2;
    if (observed(&fields, yes, no) != expected) return 3;
    if (before_store(&fields, yes, no) != expected) return 4;
    if (fields.tag != 0 || fields.rest != 12345) return 5;
  }
  return 0;
}
