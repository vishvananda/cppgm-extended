extern long compute(long);
long host_adjust(long x) { return x * 3; }
int main(void) { return compute(0) != 21 || compute(10) != 156 || compute(100) != 14871; }
