// CWG 1490/1467: an ordinary character-array expression is not the
// string literal required to construct a differently bounded array. expect reject
const char source[4] = "abc";
const char (&text)[5] = {source};
int main() { return 0; }
