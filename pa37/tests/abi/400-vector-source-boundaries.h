using Ints = int __attribute__((vector_size(16)));
using Floats = float __attribute__((vector_size(16)));
struct Box { Ints value; };
extern "C" Ints combine(Ints, Ints);
extern "C" Box add_box(Box, int);
extern "C" Floats floats(Floats, Floats);
extern "C" unsigned alignment(const void*);
