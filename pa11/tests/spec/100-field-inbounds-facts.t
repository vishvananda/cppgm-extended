struct Inner {
    int first;
    volatile int second;
};
struct Outer {
    int prefix;
    Inner inner;
};
int read_field(Outer* object) { return object->inner.first; }
void write_field(Outer* object, int value) { object->inner.second = value; }
int* zero_field(Outer* object) { return &object->prefix; }
