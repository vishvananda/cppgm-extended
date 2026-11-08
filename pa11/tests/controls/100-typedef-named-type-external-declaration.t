// expect exit 0
typedef struct { int value; } Named;
int external(Named);
int read(Named object) { return external(object); }
