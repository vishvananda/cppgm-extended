// Adopted CWG1696 example: an unused defaulted constructor is valid; selecting its temporary reference binding is ill-formed.
// expect reject
struct Holder { Holder() = default; const int& reference = 42; }; int main() { Holder value; }
