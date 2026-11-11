extern "C" int calculate(int);
extern "C" int precise(long double);
extern "C" int observe() {
 return calculate(3) == 13 && calculate(-2) == -7 && precise(1.0L) == 1 ? 17 : 2;
}
