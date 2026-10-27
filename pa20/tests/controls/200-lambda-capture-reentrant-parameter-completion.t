// expect exit 0
// N3485 focus: 5.1.2 [expr.prim.lambda]. Completing a by-value parameter
// may instantiate another body before the lambda's body is entered.
template<class T> constexpr int extent() { return 1; }
template<class T> struct Argument { int data[extent<T>()]; };
struct Holder {
    int value;
    int run() {
        auto handler = [this](Argument<char>) { ++value; };
        Argument<char> argument = {};
        handler(argument);
        return value;
    }
};
int main() {
    int count = 0;
    auto handler = [&count](Argument<int>) { ++count; };
    Argument<int> argument = {};
    handler(argument);
    Holder holder = {};
    return count != 1 || holder.run() != 1;
}
