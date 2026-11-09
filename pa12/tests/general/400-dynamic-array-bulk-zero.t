// Value initialization preserves prefix effects and clears the omitted suffix.
typedef unsigned long size_t;
alignas(long) unsigned char storage[4096];
unsigned allocations, frees, effects;
size_t requested;
void* operator new[](size_t size) {
    requested = size;
    ++allocations;
    for (size_t i = 0; i != sizeof(storage); ++i) storage[i] = 165;
    return storage;
}
void operator delete[](void*) noexcept { ++frees; }
int value() { return ++effects; }
struct Record { long first, last; };
int check(unsigned count) {
    int* values = new int[count]();
    int bad = requested != count * sizeof(int);
    for (unsigned i = 0; i != count; ++i) bad += values[i] != 0;
    delete[] values;
    return bad;
}
int prefix(unsigned count) {
    effects = 0;
    int* values = new int[count]{value(),value()};
    int bad = effects != 2 || values[0] != 1 || values[1] != 2;
    for (unsigned i = 2; i != count; ++i) bad += values[i] != 0;
    delete[] values;
    return bad;
}
int records(unsigned count) {
    Record* values = new Record[count]();
    int bad = 0;
    for (unsigned i = 0; i != count; ++i) bad += values[i].first != 0 || values[i].last != 0;
    delete[] values;
    return bad;
}
int main() {
    int bad = check(0) || check(1) || check(23) || prefix(2) || prefix(19) || records(17);
    return bad || allocations != 6 || frees != 6;
}
