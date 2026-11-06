// N3485 8.5.1p7: an omitted reference member is ill-formed;
// expect reject. CXX11-007 neighbor: the initialized first element
// cannot hide missing reference bindings in the omitted tail.
int target;
struct Element { int &reference; };
Element elements[4] = {{target}};
