constexpr char bytes[] = {'\xf0', '\237', '\xa6', '\x84', '\0'};
static_assert(__is_same(decltype('\xff'), char), "hex byte type");
static_assert(__is_same(decltype('\200'), char), "octal byte type");
static_assert('\xff' == -1 && '\x80' == -128 && '\377' == -1, "signed values");
static_assert(static_cast<unsigned char>(bytes[0]) == 240, "UTF-8 byte");
constexpr int operator "" _byte(char value) { return value; }
static_assert('\xf0'_byte == -16, "user literal argument");
template<class T> auto byte_type(T) -> decltype('\xff');
static_assert(__is_same(decltype(byte_type(0)), char), "fixed literal facts");
template<class T> struct Size { static constexpr int value = sizeof(T) + '\xff'; };
static_assert(Size<char>::value == 0, "template constant");
int main() { return 0; }
