#include <cstdio>
#include <thread>
extern "C" int locked_add(int*, int);
extern "C" int locked_compare(int*, int, int);
int main() {
    int value = 5;
    if (locked_compare(&value, 6, 9) != 5 || value != 5) return 1;
    if (locked_compare(&value, 5, 9) != 5 || value != 9) return 2;
    value = 0;
    unsigned sums[4] = {};
    std::thread workers[4];
    for (unsigned i = 0; i < 4; ++i)
        workers[i] = std::thread([&, i]() {
            for (unsigned j = 0; j < 1000; ++j) sums[i] += locked_add(&value, 1);
        });
    for (std::thread& worker : workers) worker.join();
    unsigned sum = 0;
    for (unsigned part : sums) sum += part;
    if (value != 4000 || sum != 4000u * 3999u / 2) return 3;
    std::puts("locked exchange/add and compare/exchange preserve atomic updates");
}
