// expect reject
// N3485 3.5p6: a global using target is outside the innermost enclosing namespace.
extern int n;namespace Inner{void probe(){using ::n;extern int n;}}int main(){return 0;}
