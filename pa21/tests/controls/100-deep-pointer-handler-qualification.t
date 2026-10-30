// expect exit 0
// N3485 [conv.qual]p4: adding a deep qualifier requires const at ALL
// intervening pointer levels, even when one already had const in the source.
int main() {
    int n = 7;
    int* const p = &n;
    int* const* q = &p;
    try {
        try { throw &q; }
        catch (const int* const**) { return 1; }
    } catch (int* const** result) {
        if (***result != 7) return 2;
    }
    int matched = 0;
    try { throw &q; }
    catch (const int* const* const* result) { matched = ***result == 7; }
    if (!matched) return 3;
    return 0;
}
