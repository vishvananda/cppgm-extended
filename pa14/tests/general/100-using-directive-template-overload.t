namespace Candidates {
    template<class T> int select(T, T) { return 7; }
    template<class Policy, class T> int select(T, T, Policy* = 0) { return 42; }
}
using namespace Candidates;
int select(int) { return 1; }
struct Plain {};
template<class T> struct Policy {};
int main() {
    if (select(1) != 1 || select(1,2) != 7) return 1;
    if (select<Plain>(1,2) != 42) return 2;
    if (select<Policy<int>>(1,2) != 42) return 3;
    if (Candidates::select<Policy<int>>(1,2) != 42) return 4;
    return 0;
}
