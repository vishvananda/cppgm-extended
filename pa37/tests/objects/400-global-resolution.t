extern "C" int weak_read();
extern "C" int strong_read();
extern "C" void weak_write(int);
extern "C" int initial() { return weak_read(); }
extern "C" int inspect() { return strong_read(); }
extern "C" int observe(int seed)
{
    weak_write(17 + seed);
    if (weak_read() != 17 + seed) return 1;
    if (strong_read() != 17 + seed) return 2;
    return 17;
}
