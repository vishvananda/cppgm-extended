int run(int value) { auto lambda=[](auto) { const int n=value; return n; }; return lambda(0); }
int main() { return run(7); }
