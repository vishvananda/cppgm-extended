// Adopted CWG1696 example: an unused defaulted constructor is valid; selecting its temporary reference binding is ill-formed.
// expect exit 0
struct Holder { Holder() = default; const int& reference = 42; }; int main() { return sizeof(Holder)==sizeof(void*) ? 0 : 1; }
