// N3485 8.5.4p3, 12.2p5: compact constant array images must preserve
// dynamic element initialization and required destruction. expect exit 0
int calls = 0;
int alive = 0;
int destroyed = 0;
int next() { return ++calls; }
struct Verify {
    ~Verify() { if (alive != 0 || destroyed != 3) __builtin_abort(); }
} verify;
struct Element {
    int value;
    Element() : value(++calls) { ++alive; }
    ~Element() { --alive; ++destroyed; }
};
const int (&dynamic_values)[4] = {next(), next()};
const Element (&objects)[3] = {};
int (&&mutable_values)[4] = {7, 9};
int (&alias)[4] = {mutable_values};

int main()
{
    if (calls != 5 || alive != 3 || destroyed != 0)
        return 1;
    if (dynamic_values[0] != 1 || dynamic_values[1] != 2 || dynamic_values[3] != 0)
        return 2;
    if (objects[0].value != 3 || objects[2].value != 5 || &alias != &mutable_values)
        return 3;
    alias[3] = 11;
    return mutable_values[0] != 7 || mutable_values[1] != 9 || mutable_values[3] != 11;
}
