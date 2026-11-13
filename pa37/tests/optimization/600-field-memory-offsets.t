struct Inner { int first; volatile int second; };
struct Outer { int prefix; Inner inner; };
extern "C" __attribute__((noinline)) int read_field(const Outer* p) {
    return p->inner.first;
}
extern "C" __attribute__((noinline)) void write_field(Outer* p, int value) {
    p->inner.second = value;
}
extern "C" __attribute__((noinline)) int observe_field(const Outer* p) {
    return p->inner.second + p->inner.second;
}
int main() {
    Outer object = {3, {17, 21}};
    if (read_field(&object) != 17) return 1;
    write_field(&object, 23);
    return observe_field(&object) != 46;
}
