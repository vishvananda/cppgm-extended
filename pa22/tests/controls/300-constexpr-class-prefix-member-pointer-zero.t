// N3485 5.19, 8.5, 4.11: zero class prefixes preserve typed member-pointer nulls in native data. expect exit 0
struct Owner { int x; };
struct Member { int Owner::*pointer; double floating; };
struct Zero { Member first, second; };
constexpr const Zero (&zero)[2] = {Zero()};
static_assert(zero[0].first.pointer == nullptr && zero[0].second.pointer == nullptr && zero[1].first.pointer == nullptr, "typed zero");
int main() {
    const volatile Zero* p = zero;
    return p[0].first.pointer != nullptr || p[0].second.pointer != nullptr ||
        p[1].first.pointer != nullptr || p[0].first.floating != 0.0;
}
