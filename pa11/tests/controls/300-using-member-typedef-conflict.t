// Preserve the course/GCC class-scope conflict rule for imported typedefs.
// expect reject
struct Base { typedef int Member; };
struct Derived : Base { typedef int Member; using Base::Member; };
