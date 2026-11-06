// 8.5.4p3 and 8.5.3p5: volatile qualification does not allow an lvalue
// reference to bind the character-array temporary. expect reject
const volatile char (&text)[5] = {"abc"};
int main() { return 0; }
