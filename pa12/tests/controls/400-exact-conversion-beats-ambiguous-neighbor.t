// expect exit 0
// N3485 13.3.3.1p10: an ambiguous conversion must not diagnose before ranking.
struct Source { operator int() { return 1; } operator double() { return 2; } };
int choose(long) { return 1; } int choose(Source&) { return 0; }
int main() { Source source; return choose(source); }
