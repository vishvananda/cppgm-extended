// expect reject
// N3485 focus: 9.8 [class.local], 5.1.2 [expr.prim.lambda]. The lambda's
// capture frame must not become the frame of an ordinary local class method.
template<class T> constexpr int extent() { return 1; }
template<class T> struct Argument { int data[extent<T>()]; };
int main() {
    int count = 0;
    auto handler = [&count](Argument<int>) {
        struct Local { void call() { ++count; } };
        Local local;
        local.call();
    };
    Argument<int> argument = {};
    handler(argument);
    return 0;
}
