// Empty fills must not read a null source. A wide fill's high half matters
// when deciding whether one repeated byte represents the value.
__attribute__((noinline)) void fill_bytes(char* p, const char* s, unsigned n) {
    while (n--) *p++ = *s;
}
__attribute__((noinline)) void fill_words(unsigned* p, const unsigned* s, unsigned n) {
    while (n--) *p++ = *s;
}
__attribute__((noinline)) void fill_wide(unsigned __int128* p, unsigned n) {
    while (n--) *p++ = static_cast<unsigned __int128>(1) << 64;
}
int main() {
    volatile unsigned zero = 0, two = 2;
    char bytes[4] = {'a', 'b', 'c', 0};
    unsigned words[2] = {1, 2};
    fill_bytes(bytes, nullptr, zero);
    fill_words(words, nullptr, zero);
    if (bytes[0] != 'a' || words[0] != 1 || words[1] != 2) return 1;
    const char x = 'z'; const unsigned y = 0x01010101u;
    fill_bytes(bytes, &x, two); fill_words(words, &y, two);
    if (bytes[0] != 'z' || bytes[1] != 'z' || bytes[2] != 'c' || words[0] != y || words[1] != y) return 2;
    unsigned __int128 wide[2] = {};
    fill_wide(wide, two);
    for (unsigned i = 0; i < 2; ++i)
        if (static_cast<unsigned long long>(wide[i] >> 64) != 1 || static_cast<unsigned long long>(wide[i]) != 0)
            return 3;
    return 0;
}
