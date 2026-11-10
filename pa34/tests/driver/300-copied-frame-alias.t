// A saved copy keeps its original value when a later write aliases its source.
#include <cstring>

typedef unsigned long U;

__attribute__((noinline)) U saved_value(U* target, const U* replacement, const U* source)
{
    U value = 0;
    std::memcpy(&value, source + 1, sizeof value);
    std::memcpy(target, replacement, sizeof value);
    return value;
}

int main()
{
    U state = 1;
    for (unsigned i = 0; i != 1024; ++i)
    {
        state = state * 6364136223846793005UL + 1;
        U data[2] = {state, state ^ 0x9ddfea08eb382d69UL};
        U replacement = state + 17;
        U original = data[1];
        if (saved_value(data + 1, &replacement, data) != original || data[1] != replacement)
            return 1;
        U separate = 0;
        if (saved_value(&separate, &replacement, data) != replacement || separate != replacement)
            return 2;
    }
}
