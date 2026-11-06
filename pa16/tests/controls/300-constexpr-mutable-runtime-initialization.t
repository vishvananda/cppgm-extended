// An optional constant-initialization probe must preserve a mutable read
// after an earlier dynamic write. expect exit 0
// CXX11-019 neighbor; original runtime semantics need separate adjudication.
struct A { mutable int cache; };
constexpr A source = {8};
int change() { source.cache = 9; return 0; }
int changed = change();
int observed = source.cache;
int main() { return changed != 0 || observed != 9; }
