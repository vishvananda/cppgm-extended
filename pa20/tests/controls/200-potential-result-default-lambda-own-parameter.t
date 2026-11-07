// N3485 3.2,8.3.6p9,CWG2082/2083: consume actual conversions; retain default-parameter prohibition.
// expect exit 0
int call(int value = [](int own) { return own; }(7)) { return value; } int main() { return call() - 7; }
