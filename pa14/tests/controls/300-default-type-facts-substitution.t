// N3485 14.1p9, 14.6p10: qualified default types retain declaration-time lookup,
// then substitute earlier arguments independently for each specialization.
// expect exit 0
template<class T> struct Identity { typedef T type; };
template<class T = int, class P = T*, class U = typename Identity<P>::type,
         class V = U>
struct Defaults { typedef V result; };
char choose(int);
long choose(...);
template<class T, class U = decltype(choose(T()))>
struct Selection { typedef U result; };
double choose(double);
int main() {
    Defaults<>::result p = 0;
    Defaults<long>::result q = 0;
    Selection<double>::result r = 0;
    return sizeof(*p) == sizeof(int) && sizeof(*q) == sizeof(long)
        && sizeof(r) == sizeof(char) ? 0 : 1;
}
