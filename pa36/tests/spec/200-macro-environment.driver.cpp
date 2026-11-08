#include <cstdio>
extern "C" long macro_earlier();
extern "C" long macro_later();
extern "C" long macro_raw_earlier();
int main() {
    if (macro_earlier()!=43 || macro_later()!=73 || macro_raw_earlier()!=43) return 1;
    std::puts("assembly macro snapshots retain earlier definitions and typed symbol identities");
}
