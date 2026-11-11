#if defined(__OPTIMIZE__) || !defined(__NO_INLINE__) || defined(__OPTIMIZE_SIZE__)
#error Unoptimized compilation must advertise its selected mode
#endif
int main() { return 0; }
