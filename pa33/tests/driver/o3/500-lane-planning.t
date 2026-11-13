#include "500-lane-planning.h"

int main()
{
    unsigned seeds[] = {0, 17, 0x7fffffffu, 0x80000000u, 0xffffffffu};
    for (unsigned i = 0; i < 5; ++i)
        if (planning_observe(seeds[i]) != 17) return 1;
    return 0;
}
