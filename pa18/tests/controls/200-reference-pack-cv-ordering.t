// expect exit 0
// N3485 14.8.2.4p9-p10: reference cv ordering applies to each expanded parameter.
template<class Tag, class... T> int choose(T&...) { return 1; }
template<class Tag, class... T> int choose(const T&...) { return 2; }
int main() { const int a = 1; const short b = 2; return choose<void>(a) == 2 && choose<void>(a, b) == 2 ? 0 : 1; }
