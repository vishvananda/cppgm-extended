extern "C" int read_globals();
extern "C" int ref_roundtrip(int);
extern "C" int table_roundtrip();
extern "C" int observe(int seed)
{
    if (read_globals() != 1) return 1;
    if (ref_roundtrip(seed) != seed + 17) return 2;
    if (table_roundtrip() != 1) return 3;
    return 17;
}
