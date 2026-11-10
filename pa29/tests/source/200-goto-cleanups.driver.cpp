#include <cstdio>
extern "C" int goto_templates(int);
extern "C" int goto_all_gprs();
int main() {
    for (int value = -23; value < 44; ++value)
        if (goto_templates(value) != (value >= 3) + 2 * (value >= 17)) return 2;
    if (goto_all_gprs() != 37) return 3;
    std::puts("asm exits preserve C++ cleanup, template labels and clobber-all freedom");
}
