// CWG 1490/1467 and 8.5.2p1: a narrow string cannot initialize a
// character-array temporary of another encoding kind. expect reject
const char16_t (&text)[5] = {"abc"};
int main() { return 0; }
