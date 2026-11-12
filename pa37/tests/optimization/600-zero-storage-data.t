static volatile unsigned char zeroes[32768];
static volatile unsigned char prefix[1024] = {3, 0, 5};
static volatile unsigned char tail[8] = {0, 0, 0, 0, 0, 0, 0, 9};
static volatile unsigned char* last = zeroes + 32767;
extern "C" int missing() __attribute__((weak));
static int (*absent)() = missing;
extern "C" int callback() { return 17; }
static int (*call)() = callback;

extern "C" int observe(int previous) {
 if (zeroes[0] != 0 || zeroes[16384] != 0 || *last != previous) return 1;
 if (prefix[0] != 3 || prefix[1] != 0 || prefix[2] != 5 || prefix[1023] != 0) return 2;
 if (tail[0] != 0 || tail[7] != 9 || absent != 0 || call == 0) return 3;
 *last = previous + 1;
 if (zeroes[32767] != previous + 1) return 4;
 return call();
}
