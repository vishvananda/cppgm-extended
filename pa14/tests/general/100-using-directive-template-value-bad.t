namespace Candidates { template<class T> int select(T) { return 42; } }
using namespace Candidates;
int select = 0;
int main() { return select<int>(1); }
