#include <cstdio>
// N3485 12.8p31: permitted elision keeps selected effects and destination lifetime.
int made, copied, moved, destroyed, choices;
struct Failure {};
struct Value {
    int payload;
    explicit Value(int n) : payload(n) { if (n < 0) throw Failure(); ++made; }
    Value(const Value& v) : payload(v.payload) { ++copied; }
    Value(Value&& v) : payload(v.payload) { ++moved; }
    ~Value() { ++destroyed; }
};
bool choose(bool value) { ++choices; return value; }
int check(bool left) {
    made = copied = moved = destroyed = choices = 0;
    {
        Value v = choose(left) ? Value(17) : Value(29);
        if (choices != 1 || made != 1 || v.payload != (left ? 17 : 29) ||
            copied != 0 || moved > 1 || destroyed != moved) return 1;
    }
    if (destroyed != made + copied + moved) return 2;
    made = copied = moved = destroyed = choices = 0;
    try {
        Value v = choose(left) ? Value(-1) : Value(-2);
        return 3;
    } catch (const Failure&) {
        if (choices != 1 || made != 0 || copied != 0 || moved != 0 || destroyed != 0) return 4;
    }
    {
        Value original(41);
        Value required = original;
        if (required.payload != 41 || copied != 1) return 5;
    }
    return destroyed != made + copied + moved;
}
int main() { return check(false) || check(true); }
