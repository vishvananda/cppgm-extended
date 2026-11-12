extern "C" double sin(double);
extern "C" double cos(double);
extern "C" float sinf(float);
extern "C" float cosf(float);
extern "C" double both(double value) { return sin(value) + cos(value); }
extern "C" float narrow(float value) { return sinf(value) + cosf(value); }
