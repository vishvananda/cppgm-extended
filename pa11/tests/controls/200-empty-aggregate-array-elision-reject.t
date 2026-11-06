// N3485 8.5.1p8,12: brace elision cannot initialize an empty aggregate;
// expect reject. CXX11-007 neighbor: an inferred bound must not loop
// forever when an element consumes no initializer.
struct Empty {};
Empty elements[] = {1};
