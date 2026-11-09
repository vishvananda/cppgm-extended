// N3485 3.7.4.1p1: allocation templates remain custom functions after deduction.
// Their specializations do not displace the reserved non-template placement forms.
// expect exit 0
typedef unsigned long size_t;
alignas(long) unsigned char storage[128];
int allocations;
size_t requested;
template<class T> void* operator new(size_t size, T* pointer) {
    ++allocations;
    requested = size;
    return static_cast<unsigned char*>(static_cast<void*>(pointer)) + 16;
}
template<class T> void* operator new[](size_t size, T* pointer) {
    ++allocations;
    requested = size;
    return static_cast<unsigned char*>(static_cast<void*>(pointer)) + 64;
}
struct Value { long payload; };
int main() {
    void* raw = storage;
    Value* value = new(raw) Value{17};
    if (allocations != 1 || requested != sizeof(Value) ||
        reinterpret_cast<unsigned char*>(value) != storage + 16 || value->payload != 17) return 1;
    long* array = new(raw) long[3]{23,29,31};
    return allocations != 2 || requested != 3 * sizeof(long) ||
        reinterpret_cast<unsigned char*>(array) != storage + 64 ||
        array[0] != 23 || array[1] != 29 || array[2] != 31 || value->payload != 17;
}
