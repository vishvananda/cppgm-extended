// N3485 3.6.2, 5.19: an initializer that is not constant keeps its
// ordered runtime calls and conversions. expect exit 0
int target = 7;
int calls = 0;
int* next() { ++calls; return &target; }
struct Pointer {
    operator int*() const { ++calls; return &target; }
};
int* values[4] = {next(), nullptr, next()};
int* converted[2] = {Pointer{}};
int observed = calls;

int main()
{
    return calls != 3 || observed != 3 || values[0] != &target ||
        values[1] != nullptr || values[2] != &target || values[3] != nullptr ||
        converted[0] != &target || converted[1] != nullptr;
}
