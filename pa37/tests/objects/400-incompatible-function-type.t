extern "C" double incompatible(double);
extern "C" double observe() { return incompatible(2.0); }
