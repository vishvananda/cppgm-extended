// expect reject
// N3485 focus: 14.8.2.4 [temp.deduct.partial].
// const and volatile are incomparable; reference ordering must preserve ambiguity.
struct Source {};
struct Target {
    template<class T> Target(const T&) {}
    template<class T> Target(volatile T&) {}
};
Target convert(const volatile Source& source) { return source; }
int main() {
    const volatile Source source = {};
    Target target = convert(source);
}
