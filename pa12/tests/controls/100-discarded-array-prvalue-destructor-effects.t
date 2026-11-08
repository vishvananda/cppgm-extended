// expect exit 0
// I13: Discarded class-array temporaries are constructed and destroyed at the full-expression boundary.
int live=0, dead=0;
struct A { A() { ++live; } ~A() { --live; ++dead; } };
int main() { typedef const A Array[2]; Array{}; return live!=0 || dead!=2; }
