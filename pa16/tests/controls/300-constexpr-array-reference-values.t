// N3485 5.19p2, 7.1.5p9, 8.5.4p3, 12.2p5: evaluate the typed array
// temporary, preserve its identity and bind its constant reference. expect exit 0
constexpr const int (&values)[3] = {1, 2};
constexpr const int (&single)[3] = {7};
constexpr const int (&empty)[3] = {};
constexpr const int (&direct)[3] {4, 5};
constexpr const int (&nested)[2][3] = {{1, 2}, {3}};
const int (&ordinary)[3] = {8, 9};
constexpr int ordinary_value = ordinary[1];
constexpr int existing[3] = {6, 7};
constexpr const int (&alias)[3] = {existing};
constexpr const int (&direct_alias)[3] {existing};
constexpr const int &scalar{11};
constexpr int get(const int (&v)[3]) { return v[1]; }
static_assert(values[0] == 1 && values[1] == 2 && values[2] == 0, "prefix and tail");
static_assert(single[0] == 7 && single[2] == 0 && empty[2] == 0, "one and empty");
static_assert(direct[1] == 5 && direct[2] == 0, "direct list");
static_assert(nested[0][2] == 0 && nested[1][0] == 3 && nested[1][2] == 0, "rows");
static_assert(ordinary_value == 9 && scalar == 11, "constant reads");
static_assert(&values[0] != &single[0], "distinct temporaries");
static_assert(&alias[0] == &existing[0] && &direct_alias[0] == &existing[0], "alias identity");
static_assert(get({3, 4}) == 4, "parameter binding");
int main() { return values[1] != 2 || ordinary[1] != 9 || direct[1] != 5; }
