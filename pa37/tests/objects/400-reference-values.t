extern "C" int table_roundtrip();
extern "C" int local_refs(int);
extern "C" int global_target(int value) { return value + 17; }
using Function = int (*)(int);
extern "C" Function address_ref() { return global_target; }
extern "C" int observe(int value)
{
    if (local_refs(value) != 1) return 1;
    if (table_roundtrip() != 1) return 2;
    return 17;
}
