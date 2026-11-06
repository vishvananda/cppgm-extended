// N3485 8.5.1p7, 12.4: every omitted class element is constructed
// at its own address and destroyed in reverse order. expect exit 0
int made;
int destroyed;
int bad;
struct Element {
    int id;
    int value;
    Element(int v = 0) : id(++made), value(v) {}
    ~Element() { if (id != made - destroyed) bad = 1; ++destroyed; }
};
struct Text {
    volatile char letters[65];
    Text() : letters{"text"} {}
};
struct Row { int values[65]; };
int first() { return 17; }
int main()
{
    char plain_text[65] = "text";
    if (plain_text[0] != 't' || plain_text[3] != 't' || plain_text[4] != 0 || plain_text[64] != 0)
        return 5;
    Row rows[9] = {first()};
    if (rows[0].values[0] != 17 || rows[0].values[64] != 0 || rows[8].values[64] != 0)
        return 4;
    Text text;
    if (text.letters[0] != 't' || text.letters[3] != 't' || text.letters[64] != 0)
        return 3;
    {
        Element values[33] = {{7}};
        if (made != 33 || destroyed || values[0].value != 7 || values[32].value != 0)
            return 1;
        for (int i = 0; i < 33; ++i)
            if (values[i].id != i + 1) return 2;
    }
    return made != destroyed || bad;
}
