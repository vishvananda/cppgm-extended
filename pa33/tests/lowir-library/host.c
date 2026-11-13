extern long compute(long);
long host_adjust(long x) { return x * 3; }
int main(void)
{
    for (long n = -10; n < 501; ++n) {
        long expected = 7;
        for (long i = 0; i < n; ++i) expected += i;
        if (compute(n) != expected * 3) return 1;
    }
    return 0;
}
