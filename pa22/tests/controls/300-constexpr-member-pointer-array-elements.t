// N3485 5.19 and Itanium 2.3: scalar initializer values retain
// member-pointer addresses and ABI null representations. expect exit 0
struct Holder { int value; int read() const { return value; } };
using Function = int (Holder::*)() const;
constexpr int Holder::* data_pointer() { return &Holder::value; }
constexpr Function function_pointer() { return &Holder::read; }
int Holder::* data[4] = {data_pointer(), nullptr};
Function functions[4] = {function_pointer(), nullptr};

int main()
{
    Holder object = {7};
    return object.*data[0] != 7 || (object.*functions[0])() != 7 ||
        data[1] != nullptr || data[3] != nullptr ||
        functions[1] != nullptr || functions[3] != nullptr;
}
