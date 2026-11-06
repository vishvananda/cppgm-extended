// CWG 1490/1467: an extra braced scalar element does not initialize
// the whole character-array temporary from a literal. expect reject
const char (&text)[5] = {{"abc"}};
int main() { return 0; }
