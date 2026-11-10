// Recorded scalar constant initializers still materialize addressable and
// volatile storage, while nonconstant object expressions keep their effects.
#include <cstdio>
#include <limits>
#include <cstdint>

struct Property { constexpr int value() const { return 42; } };
static unsigned calls;
__attribute__((noinline)) Property object() { ++calls; return Property(); }
__attribute__((noinline)) long runtime_value(long n) { ++calls; return n; }
__attribute__((noinline)) long read(const long* value) { return *value; }

template<class T>
__attribute__((noinline)) bool fits(long value) {
    const long low = long(std::numeric_limits<T>::min());
    const unsigned long high = static_cast<unsigned long>(std::numeric_limits<T>::max());
    return value >= low && (value < 0 || static_cast<unsigned long>(value) <= high);
}

int main() {
    const long low = long(std::numeric_limits<long>::min());
    const long high = long(std::numeric_limits<long>::max());
    const volatile long stored = long(std::numeric_limits<long>::max());
    unsigned bad = read(&low) != std::numeric_limits<long>::min();
    bad += read(&high) != std::numeric_limits<long>::max();
    bad += stored != high;
    const int property = object().value();
    const long dynamic = long(runtime_value(17));
    bad += property != 42 || dynamic != 17 || calls != 2;
    uint64_t hash = 0;
    for (long i = -1000; i < 1000; ++i)
        hash = hash * 31 + fits<signed char>(i) + 3 * fits<unsigned short>(i) + 7 * fits<long>(i);
    std::printf("%u %u %llu\n", bad, calls, static_cast<unsigned long long>(hash));
    return bad != 0;
}
