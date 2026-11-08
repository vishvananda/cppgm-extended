// expect exit 0
// Adopted CWG2352: the compatible reference denotes the original pointer object.
int main() { int n = 3; int* p = &n; const int* const& r = p; p = 0; return r == 0 ? 0 : 1; }
