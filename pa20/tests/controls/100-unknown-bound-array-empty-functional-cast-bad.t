// expect reject
// N3485 8.5.1p4: a functional array temporary cannot deduce its bound from an empty list.
typedef int Unknown[]; int main() { Unknown{}; }
