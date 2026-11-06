// 8.5.4p3 and 8.5.3p5: a mutable lvalue reference cannot bind the
// character-array temporary initialized by the string list. expect reject
char (&text)[5] = {"abc"};
int main() { return 0; }
