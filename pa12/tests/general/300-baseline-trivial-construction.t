// N3485 12.8p16, 18.6.1.3: trivial construction transfers values; reserved placement returns its argument.
// Trivial transfers keep source effects, destruction, and overlapping base padding.
typedef unsigned long size_t;
void* operator new(size_t, void* p) noexcept { return p; }
void* operator new[](size_t, void* p) noexcept { return p; }
unsigned effects, destroyed;
struct Trivial { long value; Trivial() = default; Trivial(Trivial&&) = default; };
struct Plain { long value; };
struct Empty { ~Empty() { ++destroyed; } };
Plain source = {73};
Plain& source_effect() { ++effects; return source; }
void* storage_effect(void* p) { ++effects; return p; }
struct Custom {
    int value;
    static void* operator new(size_t, void* p) noexcept { ++effects; return p; }
};
struct Variadic { long value; };
void* operator new(size_t, void* p, int, ...) noexcept { ++effects; return p; }
struct Base { int value; char tag; ~Base() {} };
struct Derived : Base { char sentinel; };
int run() {
    unsigned bad = 0;
    Trivial first; first.value = 29;
    Trivial second(static_cast<Trivial&&>(first));
    bad += second.value != 29;
    alignas(Plain) unsigned char storage[sizeof(Plain)];
    Plain* copied = new(storage_effect(storage)) Plain(source_effect());
    bad += copied != reinterpret_cast<Plain*>(storage) || copied->value != 73 || effects != 2;
    alignas(Empty) unsigned char empty_storage[sizeof(Empty)];
    { Empty original; Empty* copy = new(empty_storage) Empty(original); copy->~Empty(); }
    bad += destroyed != 2;
    alignas(Custom) unsigned char custom_storage[sizeof(Custom)];
    Custom* custom = new(custom_storage) Custom{51};
    bad += custom->value != 51 || effects != 3;
    alignas(Variadic) unsigned char variadic_storage[sizeof(Variadic)];
    // Explicit third placement argument selects the custom variadic overload.
    Variadic* variadic = new(variadic_storage, 7) Variadic{91};
    bad += variadic->value != 91 || effects != 4;
    alignas(long) unsigned char array_storage[3 * sizeof(long)];
    long* array = new(storage_effect(array_storage)) long[3]{2,3,5};
    bad += array != reinterpret_cast<long*>(array_storage) || array[0]+array[1]+array[2] != 10 || effects != 5;
    Derived d; d.value = 1; d.tag = 2; d.sentinel = 99;
    Base b; b.value = 47; b.tag = 8;
    Base* base = new(static_cast<void*>(static_cast<Base*>(&d))) Base(b);
    bad += base->value != 47 || base->tag != 8 || d.sentinel != 99;
    return bad != 0;
}
int main() { return run(); }
