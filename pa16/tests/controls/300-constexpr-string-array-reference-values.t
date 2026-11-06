// CWG 1490/1467 and 5.19: string-initialized array temporaries keep
// their code units and zero padding in constant evaluation. expect exit 0
// CXX11-021; the object-valued constexpr neighbor belongs to PA16:300.
constexpr const char (&text)[5] = {"abc"};
constexpr int end(const char (&value)[5]) { return value[2] + value[4]; }
static_assert(text[0] == 'a' && text[3] == 0 && text[4] == 0, "static array value");
static_assert(end({"abc"}) == 'c', "temporary array argument");
int main() { return text[2] != 'c' || text[4] != 0 || end({"abc"}) != 'c'; }
