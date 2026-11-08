// Compile-only: an external definition can be supplied by another translation unit.
typedef struct { int value; } Named;
int external(Named);
int read(Named object) { return external(object); }
