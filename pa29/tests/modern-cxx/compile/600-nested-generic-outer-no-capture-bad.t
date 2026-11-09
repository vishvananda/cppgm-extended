template<class T>int run(T value) {
  auto outer=[] { auto inner=[&](auto n) { return value+n; }; return inner(2); };
  return outer();
}
int main() { return run(5); }
