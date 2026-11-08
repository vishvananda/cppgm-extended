#include <cstdio>
extern "C" void repeated_copy(unsigned char*, const unsigned char*, unsigned long);
int main() {
    unsigned char source[80], destination[80];
    for (unsigned i = 0; i < 80; ++i) source[i] = static_cast<unsigned char>(i * 17 + 3);
    for (unsigned count = 0; count < 76; ++count) {
        for (unsigned i = 0; i < 80; ++i) destination[i] = 0xa5;
        repeated_copy(destination + 2, source + 1, count);
        for (unsigned i = 0; i < 80; ++i) {
            unsigned expected = i >= 2 && i < count + 2 ? source[i - 1] : 0xa5;
            if (destination[i] != expected) return 1;
        }
    }
    std::puts("REP string copies preserve zero-count behavior and guard bytes");
}
