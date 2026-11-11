volatile int observations = 0;
int initial_value() { ++observations; return 37; }
int initialized = initial_value();
extern "C" int initialized_value() { return initialized + observations; }
