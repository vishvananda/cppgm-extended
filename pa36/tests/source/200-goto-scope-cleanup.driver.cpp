#include <cstdio>
extern "C" int goto_cleanup(int);
int main() {
    if (goto_cleanup(0) != 111 || goto_cleanup(1) != 11) return 1;
    std::puts("outgoing assembly exits destroy the C++ objects whose scope they leave");
}
