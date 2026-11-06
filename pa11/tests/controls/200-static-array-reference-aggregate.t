// N3485 8.5.4p3, 8.5.1p7: array-reference temporaries use the aggregate
// elements' static data, padding and writable mutable members. expect exit 0
struct Pair { int value; const char* text; };
const Pair (&pairs)[4] = {{3, "hi"}, {5, "x"}};
struct State { mutable int value; };
const State (&states)[2] = {{7}};

int main()
{
    if (pairs[0].value != 3 || pairs[0].text[1] != 'i' || pairs[1].value != 5)
        return 1;
    if (pairs[2].value != 0 || pairs[3].text != nullptr || states[1].value != 0)
        return 2;
    states[0].value = 9;
    return states[0].value != 9;
}
