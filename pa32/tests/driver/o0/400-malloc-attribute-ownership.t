extern "C" void dispose(void*);
extern "C" {
__attribute__((malloc)) void* plain(unsigned long);
__attribute__((__malloc__, returns_nonnull)) void* reserved(unsigned long);
[[gnu::malloc]] void* scoped(unsigned long);
void* later(unsigned long);
__attribute__((malloc)) void* later(unsigned long);
// Only a deallocator association: this may return existing storage.
__attribute__((malloc(dispose, 1))) void* association(unsigned long);
void* invoke_plain(unsigned long n) { return plain(n); }
void* invoke_reserved(unsigned long n) { return reserved(n); }
void* invoke_scoped(unsigned long n) { return scoped(n); }
void* invoke_later(unsigned long n) { return later(n); }
void* invoke_association(unsigned long n) { return association(n); }
}
template<class T> __attribute__((malloc)) T* produce(unsigned long);
template<> int* produce<int>(unsigned long);
double* implicit_instance(unsigned long n) { return produce<double>(n); }
int* explicit_instance(unsigned long n) { return produce<int>(n); }
struct Factory { virtual void* get(unsigned long) __attribute__((malloc)); };
void* virtual_instance(Factory* factory, unsigned long n) { return factory->get(n); }
void* function_pointer(void* (*fn)(unsigned long), unsigned long n) { return fn(n); }
