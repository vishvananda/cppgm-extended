#include <cstdio>
extern "C" void direct_exit_tags(int*);
int main() {
    int values[5] = {-1,-1,-1,83,97};
    direct_exit_tags(values);
    if (values[0] != 17 || values[1] != 41 || values[2] != 0 ||
        values[3] != 83 || values[4] != 97) return 1;
    std::puts("frontend-neutral exit tags need no spare register and retain invocation identities");
}
