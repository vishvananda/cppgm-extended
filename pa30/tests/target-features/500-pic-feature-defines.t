#if __PIC__ != 2 || __pic__ != 2 || defined(__PIE__) || defined(__pie__)
#error PIC must not inherit the host's PIE mode
#endif
int main() { return 0; }
