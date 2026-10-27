// VALIDATION: compile-fail
// Neither lookup supplies the object's type.
struct Object { typedef int Alias; };
typedef long Alias;
void destroy(Object* p) { p->~Alias(); }
