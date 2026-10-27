// expect exit 0
// N3485 14.8.2p8: a non-class functional cast with several arguments
// fails immediate substitution; class construction remains a viable candidate.
struct Pair { Pair(int, int) {} };
template<class T, class = decltype(T(1, 2))> int by_default(T*) { return 1; }
int by_default(...) { return 0; }
template<class T> auto by_result(T*) -> decltype(T(1, 2), int()) { return 1; }
int by_result(...) { return 0; }
int main() {
    return by_default((Pair*)0) != 1 || by_default((int*)0) != 0 ||
        by_result((Pair*)0) != 1 || by_result((int*)0) != 0 ||
        by_default((void*)0) != 0;
}
