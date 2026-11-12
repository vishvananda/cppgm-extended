static int initializations;
static int local_initializations;
static int initialize() { ++initializations; return 40; }
static int state = initialize();
extern "C" int inspect() { return state + 100 * initializations + 1000 * local_initializations; }
extern "C" int step(int delta) { state += delta; return state; }
static int local_value() { ++local_initializations; return 10; }
extern "C" int local_step() { static int value = local_value(); return ++value; }
extern "C" long long echo(long long value) { return value; }
extern "C" double floating(double value) { return value + 0.25; }
