// N3485 8.5.4p3 and Itanium 2.3: a published constant array temporary
// retains both a zero-offset member and the -1 null tail. expect exit 0
struct Element { int value; };
using Member = int Element::*;
struct Holder { Member const (&values)[33]; };
constexpr Holder holder = {{&Element::value}};
struct Record { Member pointer; };
constexpr const Record (&records)[33] = {};
static_assert(holder.values[0] == &Element::value, "non-null zero offset");
static_assert(holder.values[32] == nullptr, "null omitted tail");
static_assert(records[0].pointer == nullptr && records[32].pointer == nullptr, "aggregate null tail");
int main()
{
    long long bits = 0;
    __builtin_memcpy(&bits, &holder.values[32], sizeof(bits));
    long long aggregate_bits = 0;
    __builtin_memcpy(&aggregate_bits, &records[32].pointer, sizeof(aggregate_bits));
    Element value{7};
    return bits != -1 || aggregate_bits != -1 || value.*holder.values[0] != 7;
}
