// CWG 1467: suitably typed literals have identity conversions even when
// a selected target would be too short. The call is ambiguous. expect reject
// CXX11-021; do not discard the shorter candidate during ranking.
int choose(const char (&)[3]);
int choose(const char (&)[5]);
int main() { return choose({"abc"}); }
