extern "C" __attribute__((returns_nonnull)) int* required(int*);
extern "C" int* optional(int*);
template<class T> __attribute__((returns_nonnull)) T* templated(T*);
template<> int* templated<int>(int*);
struct Interface { virtual int* fetch() __attribute__((returns_nonnull)); };
extern "C" {
bool checked(int* p) { return required(p) != nullptr; }
bool unchecked(int* p) { return optional(p) != nullptr; }
bool checked_template(double* p) { return templated(p) != nullptr; }
bool unchecked_specialization(int* p) { return templated(p) != nullptr; }
bool unchecked_pointer(int* (*fn)(int*), int* p) { return fn(p) != nullptr; }
bool checked_virtual(Interface* object) { return object->fetch() != nullptr; }
}
