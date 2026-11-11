extern "C" int missing(int);
extern "C" int unused() { return missing(4); }
extern "C" int observe() { return 31; }
