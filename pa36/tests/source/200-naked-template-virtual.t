struct First { virtual long first() const { return 1; } };
struct Second { virtual const void* second() const = 0; };
struct Both : First, Second {
  __attribute__((naked)) const void* second() const override { asm("mov %rdi, %rax; ret"); }
};
__attribute__((noinline)) const void* indirect(Second* object) { return object->second(); }
extern "C" long naked_virtual() { Both value; return indirect(&value) == &value ? 42 : -1; }
template<class T> __attribute__((naked)) long raw_template(T) { asm("lea 9(%rdi),%rax; ret"); }
extern "C" long naked_template(long value) { return raw_template(value) + 15; }
