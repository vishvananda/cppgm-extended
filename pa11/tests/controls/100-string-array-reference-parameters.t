// CWG 1490/1467: explicit and default list arguments construct the whole
// character array; a related array still binds directly. expect exit 0
// CXX11-021.
int first(const char (&text)[5] = {"abc"}) { return text[0] + text[4]; }
int same(const char (&text)[4], const char* original) { return &text[0] == original; }

int main()
{
    const char source[4] = "abc";
    return first() != 'a' || first({"xyz"}) != 'x' || !same({source}, source);
}
