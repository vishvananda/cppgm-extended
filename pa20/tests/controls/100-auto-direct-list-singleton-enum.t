// expect exit 0
// I13: N3922: singleton direct-list auto deduces its element, including nondependent template bodies.
enum class Flag { yes=7 };
Flag choose() { return Flag::yes; }
template<class T> int unused() { const auto item{choose()}; return item != Flag::yes; }
int main() { const auto item{choose()}; auto plain=choose(); return item!=Flag::yes || plain!=Flag::yes || unused<int>(); }
