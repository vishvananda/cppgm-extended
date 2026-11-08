#include <cstdio>
extern "C" void escaped_cfa_expression(void (*)());
extern "C" void escaped_cfa_bookkeeping(void (*)());
int calls = 0;
void returning() { ++calls; }
void throwing() { ++calls; throw 97; }
int main() {
    void (*functions[])(void (*)()) = {escaped_cfa_expression, escaped_cfa_bookkeeping};
    for (unsigned i=0;i<2;++i) {
        functions[i](returning);
        bool caught = false;
        try { functions[i](throwing); }
        catch (int value) { caught = value==97; }
        if (!caught) return 1;
    }
    if (calls!=4) return 2;
    std::puts("escaped CFA expressions and GNU logical CFA bookkeeping passed");
}
