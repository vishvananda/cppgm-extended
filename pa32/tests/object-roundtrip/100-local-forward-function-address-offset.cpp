// Streamed selection must know the internal linkage of a forward function.
// The offset body is compiled, but is not called by the program.
static int local(int);
char* local_offset() { return reinterpret_cast<char*>(&local) + 1; }
static int local(int value) { return value + 2; }
int main() { return 0; }
