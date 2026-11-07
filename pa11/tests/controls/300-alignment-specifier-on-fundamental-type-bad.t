// N3485 7.1p1, 7.6.1p5 and 7.6.2p1: post-specifier alignas appertains to int, not its variable.
// expect reject
int alignas(8) value;
int main() { return 0; }
