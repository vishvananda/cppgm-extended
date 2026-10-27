// VALIDATION: compile-fail
// A qualified destructor keeps lookup in the named class.
struct Object { typedef int Alias; };
typedef Object Alias;
void destroy(Object* p) { p->Object::~Alias(); }
