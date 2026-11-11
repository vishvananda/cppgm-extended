#if __PIC__ != 1 || __pic__ != 1 || __PIE__ != 1 || __pie__ != 1
#error The last PIC or PIE selection must control the advertised mode
#endif
int main() { return 0; }
