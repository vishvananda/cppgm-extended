extern "C" int missing_integer(int) __attribute__((weak));
extern "C" double missing_floating(double, long long) __attribute__((weak));
static int (*integer_address)(int) = missing_integer;
static double (*floating_address)(double, long long) = missing_floating;

extern "C" int guarded() {
 return missing_integer ? missing_integer(9) : 17;
}
extern "C" int null_addresses() {
 return integer_address == 0 && floating_address == 0 ? 17 : 2;
}
extern "C" int force_integer() { return missing_integer(9); }
extern "C" double force_floating() { return missing_floating(2.5, 19); }
