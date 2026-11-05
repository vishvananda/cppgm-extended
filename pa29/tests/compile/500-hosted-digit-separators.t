#define JOIN(a, b) a##b
#define TEXT(a) #a

#if 1'024 != 1024 || 0b1010'0011 != 163
#error digit separators in controlling expressions
#endif

static_assert(1'000'000 == 1000000, "decimal");
static_assert(07'71 == 505, "octal");
static_assert(0xAB'CD == 43981, "hexadecimal");
static_assert(0b1010'0011 == 163, "binary");
static_assert(1'2.5e1'0 == 125000000000.0, "decimal floating sequences");
static_assert(0x1'f.8p1 == 63.0, "hexadecimal floating sequences");
static_assert(JOIN(1'2, 3) == 123, "token pasting keeps one pp-number");
static_assert(TEXT(1'024)[1] == '\'', "stringizing preserves spelling");

constexpr unsigned long long operator"" _cooked(unsigned long long n) { return n; }
static_assert(1'024_cooked == 1024, "cooked literal value");

template<char... Chars> constexpr unsigned operator"" _count() { return sizeof...(Chars); }
constexpr unsigned length = 123'456_count;
static_assert(length == 7, "raw literal spelling includes separators");
constexpr unsigned operator"" _raw(const char* text) { return text[3] == '\'' ? 1 : 0; }
static_assert(123'456_raw == 1, "raw literal text includes separators");
