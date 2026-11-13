#include "500-conditional-vector-reductions.h"

int main()
{
    for (unsigned n = 0; n <= 83; ++n)
        if (observe(n * 0x1234567u, n) != 17) return 1;
    return 0;
}
