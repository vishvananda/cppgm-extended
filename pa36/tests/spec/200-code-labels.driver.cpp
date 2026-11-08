#include <cstdio>
extern "C" int shared_dispatch(long, int);
int main() {
    for (int n = -30; n <= 30; ++n)
        if (shared_dispatch(0, n) != n + 13 || shared_dispatch(1, n) != n * 7) return 1;
    std::puts("frontend-neutral code label dispatch passed");
}
