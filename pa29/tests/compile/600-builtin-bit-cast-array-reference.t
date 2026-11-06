// Hosted builtin compatibility: const array qualifiers come from the
// elements, and representation queries include the omitted tail. expect exit 0
constexpr const unsigned char (&bytes)[8] = {1};
constexpr unsigned long long value = __builtin_bit_cast(unsigned long long, bytes);
constexpr unsigned long long from_parameter(const unsigned char (&input)[8])
{
    return __builtin_bit_cast(unsigned long long, input);
}
static_assert(value == 1, "little-endian array temporary");
static_assert(from_parameter({2}) == 2, "array parameter representation");
struct Bytes { unsigned char values[8]; };
constexpr const Bytes &wrapped = static_cast<const Bytes>(Bytes{{3}});
constexpr unsigned long long member = __builtin_bit_cast(unsigned long long, wrapped.values);
static_assert(member == 3, "const member array of a class temporary");
constexpr const Bytes (&objects)[2] = {{{1}}, {{2}}};
constexpr unsigned long long first = __builtin_bit_cast(unsigned long long, objects[0]);
constexpr int clobber(const int (&values)[64]) { return values[0]; }
constexpr int marker = clobber({7});
constexpr unsigned long long second = __builtin_bit_cast(unsigned long long, objects[0]);
static_assert(first == 1 && second == 1 && objects[0].values[7] == 0 && objects[1].values[0] == 2,
              "encoding a nested prefix preserves published storage");
int main() { return value != 1 || member != 3 || first != 1 || second != 1 || marker != 7; }
