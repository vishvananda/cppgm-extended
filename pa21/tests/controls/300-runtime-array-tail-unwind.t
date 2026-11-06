// N3485 8.5.1p7, 12.2p4, 15.2p2: looped omitted constructors keep
// runtime element/row counts, default-argument temporaries and cleanup. expect exit 0
int attempts, destroyed, tokens, token_dead, remaining, fail_at, bad;
struct Token {
    Token() { ++tokens; }
    operator int() const { return tokens; }
    ~Token() { ++token_dead; }
};
struct Element {
    int id;
    Element(int value = Token()) : id(++attempts) {
        if (value != id) bad = 1;
        if (id == fail_at) throw id;
    }
    ~Element() { if (id != remaining--) bad = 1; ++destroyed; }
};
int run(int limit)
{
    attempts = destroyed = tokens = token_dead = 0;
    remaining = limit - 1;
    fail_at = limit;
    try {
        Element values[17][3] = {{{Token()}}};
        return 1;
    } catch (int value) {
        if (value != limit || attempts != limit || destroyed != limit - 1 ||
            tokens != limit || token_dead != limit || remaining != 0) return 2;
    }
    return bad;
}
int zero_destroyed;
struct Zero { int value; ~Zero() { ++zero_destroyed; } };
struct Bomb { Bomb() { throw 7; } };
struct Outer { Zero values[33]; Bomb bomb; };
int main()
{
    if (run(1) || run(4) || run(20) || run(50)) return 1;
    try { Outer object = {{}}; return 2; }
    catch (int) { return zero_destroyed != 33; }
}
