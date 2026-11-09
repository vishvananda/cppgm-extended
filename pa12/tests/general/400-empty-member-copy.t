// Trivial empty members need no value transfer; user-defined copies keep effects.
unsigned effects;
struct Empty {};
struct Plain { long first; Empty empty; long last; };
struct Effect {
    Effect() = default;
    Effect(const Effect&) { ++effects; }
    Effect& operator=(const Effect&) { ++effects; return *this; }
};
struct Nontrivial { long first; Effect empty; long last; };
void copy_plain(Plain& d, const Plain& s) { d = s; }
void copy_effect(Nontrivial& d, const Nontrivial& s) { d = s; }
int run() {
    Plain a = {17, {}, 23}, b = {};
    copy_plain(b, a);
    Plain c(a);
    Nontrivial x; x.first = 29; x.last = 31;
    Nontrivial y(x);
    copy_effect(y, x);
    return b.first != 17 || b.last != 23 || c.first != 17 || c.last != 23 ||
        y.first != 29 || y.last != 31 || effects != 2;
}
int main() { return run(); }
