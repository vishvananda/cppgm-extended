// expect reject
// N3485 13.3.3.1p10: an ambiguous user conversion stays viable ahead of ellipsis.
struct Source { operator int() { return 1; } operator double() { return 2; } };
void choose(long) {} void choose(...) {}
int main() { Source source; choose(source); }
