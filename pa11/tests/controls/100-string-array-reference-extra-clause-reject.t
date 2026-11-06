// CWG 1490/1467: the special string-array list contains one literal,
// not a string followed by another scalar initializer. expect reject
const char (&text)[5] = {"abc", 0};
int main() { return 0; }
