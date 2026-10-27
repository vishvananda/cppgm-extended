// expect exit 0
// CWG 1391: reject impossible conversions to originally nondependent
// parameters after completing deduction, before substituting the signature.
template<class T> struct Poison { typedef typename T::missing type; };
struct Item {};
template<class T> typename Poison<T>::type select(void*, T);
template<class T> int select(int, T) { return 7; }
struct Middle { Middle(int) {} };
struct Fixed { Fixed(Middle) {} };
template<class T> typename Poison<T>::type twice(Fixed, T);
template<class T> int twice(int, T) { return 9; }
template<class T> int once(Fixed, T) { return 11; }
template<class T> int list(int, T) { return 13; }
int callback(int n) { return n + 2; }
double callback(double n) { return n; }
template<class T> int invoke(int (*fn)(int), T) { return fn(3); }
template<class T, class U> int missing(Poison<Item>, T);
int missing(...) { return 15; }
template<class... Ts, class T> typename Poison<T>::type packed(Ts..., void*, T);
template<class... Ts> int packed(...) { return 17; }
int main() {
    return select(1, Item()) != 7 || twice(1, Item()) != 9 ||
        once(Middle(1), Item()) != 11 || list({3}, Item()) != 13 ||
        invoke(callback, Item()) != 5 || missing(1, Item()) != 15 ||
        packed<int>(1, 2, Item()) != 17;
}
