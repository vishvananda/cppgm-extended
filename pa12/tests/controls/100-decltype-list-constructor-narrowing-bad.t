// CXX11-118: ordinary (non-template) narrowing must still be diagnosed.
// N3485 8.5.4p7; neighbor of GCC cpp0x/Wnarrowing19.C.
// expect reject
struct Target { Target(int); };
double value(double);
typedef decltype(Target{value(0)}) Result;
