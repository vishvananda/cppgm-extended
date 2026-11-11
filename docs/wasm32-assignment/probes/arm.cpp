// AArch64 machine-interface research probe, not an ARM implementation.
struct FloatPair { float a, b; };
struct Quad { long a, b, c, d; };
extern "C" FloatPair floats(FloatPair a) { return a; }
extern "C" Quad aggregate(Quad a) { return a; }
extern "C" int choose(int a, int b) { return a > b ? a + 3 : b - 1; }
