#include <cstdio>
extern "C" long naked_specialization(long);
int main() {
    for (long value = -81; value < 82; ++value)
        if (naked_specialization(value) != value + 24) return 1;
    std::puts("explicit specializations state their own naked attribute");
}
