// N3485 18.6.1.3: reserved placement returns its pointer after argument evaluation.
// expect exit 0
typedef unsigned long size_t;
inline void* operator new(size_t, void* p) noexcept { return p; }
unsigned effects;
void* pointer_effect(void* p) { ++effects; return p; }
long storage;
int main() {
    long* value = new(pointer_effect(&storage)) long(47);
    return value != &storage || *value != 47 || effects != 1;
}
