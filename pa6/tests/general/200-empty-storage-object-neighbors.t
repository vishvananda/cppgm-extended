// N3485 7.1.1: with declarators, the storage specifiers apply to objects, not their class specifiers.
extern struct ExternalType { int member; } external_object;
ExternalType external_object = {1};
static struct StaticType { int member; } static_object;
thread_local struct ThreadType { int member; } thread_object;
struct Owner { mutable struct Item { int member; } item; };
void f() { register int value = 3; }
