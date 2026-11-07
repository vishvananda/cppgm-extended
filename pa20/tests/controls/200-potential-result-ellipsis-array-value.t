// N3485 3.2,8.3.6p9,CWG2082/2083: consume actual conversions; retain default-parameter prohibition.
// expect exit 0
int effects; void sink(...) {} struct Object { int data[2]; }; int main() { constexpr Object object = {{3, 5}}; [] { sink(object.data[effects++]); }(); return effects - 1; }
