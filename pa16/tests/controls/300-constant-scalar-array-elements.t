// N3485 3.6.2p2, 5.19, 7.1.5: constant initialization covers explicit
// scalar array elements, their calls and conversions. expect exit 0
int target[3] = {7, 11, 13};
constexpr int* address() { return target; }
constexpr int* advance(int* value, int count) { return count ? advance(value + 1, count - 1) : value; }
constexpr int* choose(bool selected, int* value = nullptr) { return selected ? value : nullptr; }
struct Pointer {
    int* value;
    constexpr operator int*() const { return value; }
};
extern int* values[6];
int read_before_definition() { return values[0] == target && values[1] == target + 1; }
int snapshot = read_before_definition();
int* values[6] = {address(), advance(address(), 1), choose(false, address()), Pointer{target + 2}, choose(true)};
constexpr double number() { return 1.25; }
double numbers[4] = {number(), number() + 2.0};
int* volatile volatile_values[2] = {address()};

int main()
{
    if (snapshot != 1 || *values[0] != 7 || *values[1] != 11 || *values[3] != 13)
        return 1;
    if (values[2] != nullptr || values[4] != nullptr || values[5] != nullptr)
        return 2;
    if (numbers[0] != 1.25 || numbers[1] != 3.25 || numbers[3] != 0.0)
        return 3;
    return volatile_values[0] != target || volatile_values[1] != nullptr;
}
