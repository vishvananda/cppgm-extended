// CWG 1490/1467 and the inherited parenthesized literal extension:
// parentheses preserve literal recognition and zero padding. expect exit 0
// CXX11-021.
const char (&global)[5] = {(("abc"))};
int padded(const char (&value)[5]) { return value[2] != 'c' || value[4] != 0; }
int main()
{
    const char (&local)[5]{("abc")};
    return global[0] != 'a' || global[4] != 0 || local[1] != 'b' ||
        local[4] != 0 || padded({("abc")});
}
