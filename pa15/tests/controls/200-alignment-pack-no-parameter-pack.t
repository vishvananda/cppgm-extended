// N3485 7.6.2 and 14.5.3: expand alignment patterns and keep their strictest valid requirement.
// expect reject
struct alignas(8 ...) Bad { char value; };
int main() { return 0; }
