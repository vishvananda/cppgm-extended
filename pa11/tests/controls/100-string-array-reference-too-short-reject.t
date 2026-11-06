// CWG 1490/1467 and 8.5.2p2: the target must include the literal's
// terminating null code unit. expect reject
const char (&text)[3] = {"abc"};
int main() { return 0; }
