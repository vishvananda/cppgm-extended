// N3485 8.5.4p3, 8.5p5 and Itanium 2.3: value-initialized member
// pointers in static array-reference temporaries keep ABI null values.
// A data-member null is -1, not all-zero bytes. expect exit 0
struct Holder { int value; int read() const { return value; } };
int Holder::* const (&data)[4] = {};
int (Holder::* const (&functions)[4])() const = {};
struct Members { int Holder::* data; int (Holder::* function)() const; };
const Members (&objects)[3] = {};

int main()
{
    return data[0] != nullptr || data[3] != nullptr || functions[0] != nullptr ||
        functions[3] != nullptr || objects[0].data != nullptr ||
        objects[2].data != nullptr || objects[2].function != nullptr;
}
