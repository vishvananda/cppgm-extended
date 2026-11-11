extern "C" int missing(int);
extern "C" int observe() { return missing(3); }
