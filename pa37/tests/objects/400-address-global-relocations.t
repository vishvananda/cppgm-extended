extern "C" int shared;
extern "C" int operation(int);
extern "C" int inspect(int);
extern "C" int observe() { return inspect(4) == 42 ? 17 : 1; }
