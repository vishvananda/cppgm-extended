// CWG 1490/1467 and 3.6.2p2: the reference and its constant character
// array are initialized before the earlier dynamic constructor. expect exit 0
// CXX11-021.
extern const char (&text)[5];
int observed = 1;
struct Observer {
    Observer() { observed = text[0] != 'a' || text[2] != 'c' || text[3] != 0 || text[4] != 0; }
} observer;
const char (&text)[5] = {"abc"};

int main() { return observed; }
