// expect reject
// I13: N3485 2.14.8/7.3.4: literal lookup must retain both equally viable using-directive candidates.
int operator "" _which(unsigned long long); namespace Other { int operator "" _which(unsigned long long); }
using namespace Other; int value=10_which;
