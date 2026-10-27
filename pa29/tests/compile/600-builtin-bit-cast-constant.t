// Scalar representation changes are evaluated inside constexpr calls too.
template<class To, class From>
constexpr To bits(From const& value) { return __builtin_bit_cast(To, value); }
static_assert(bits<unsigned>(-1) == ~0u, "signed bits");
static_assert(bits<int>(~0u) == -1, "unsigned bits");
static_assert(bits<unsigned>(1.0f) == 0x3f800000u, "float bytes");
static_assert(bits<float>(0x3f800000u) == 1.0f, "float value");
static_assert(bits<unsigned long long>(-0.0) == 0x8000000000000000ull, "negative zero");
static_assert(bits<double>(0x3ff0000000000000ull) == 1.0, "double value");
static_assert(bits<unsigned char>(true) == 1, "boolean bytes");
static_assert(bits<bool>(static_cast<unsigned char>(1)), "boolean value");
constexpr unsigned __int128 wide = (static_cast<unsigned __int128>(1) << 100) | 7;
static_assert(bits<__int128>(wide) == static_cast<__int128>(wide), "wide bytes");
enum class Word : unsigned { value = 0x12345678u };
static_assert(bits<unsigned>(Word::value) == 0x12345678u, "enumeration bytes");
int main() { return 0; }
