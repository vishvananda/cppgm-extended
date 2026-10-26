// expect exit 0
// N3485 focus: 13.3.1.4 [over.match.copy], 14.8.2.4 [temp.deduct.partial].
// Copy-initialization's converting constructors use reference partial ordering
// even when their deduced parameter types have identical cv-qualification.
struct Source {};
struct Target {
    int chosen;
    template<class T> Target(T&, int* = 0) : chosen(1) {}
    template<class T> Target(const T&, int* = 0) : chosen(2) {}
    template<class T> Target(volatile T&, int* = 0) : chosen(3) {}
    template<class T> Target(const volatile T&, int* = 0) : chosen(4) {}
};
Target mutable_source(Source& source) { return source; }
Target const_source(const Source& source) { return source; }
Target volatile_source(volatile Source& source) { return source; }
Target cv_source(const volatile Source& source) { return source; }
struct ReferenceTarget {
    int chosen;
    template<class T> ReferenceTarget(T&) : chosen(1) {}
    template<class T> ReferenceTarget(T&&) : chosen(2) {}
};
ReferenceTarget lvalue_source(Source& source) { return source; }
ReferenceTarget rvalue_source() { return Source(); }
int main() {
    Source source;
    const Source constant = {};
    volatile Source vol;
    const volatile Source cv = {};
    Target direct(constant);
    Target copied = constant;
    return mutable_source(source).chosen != 1 ||
           const_source(constant).chosen != 2 ||
           volatile_source(vol).chosen != 3 ||
           cv_source(cv).chosen != 4 ||
           direct.chosen != 2 || copied.chosen != 2 ||
           lvalue_source(source).chosen != 1 || rvalue_source().chosen != 2;
}
