int* advance(int* p, long n) { return p + n; }
int* retreat(int* p, long n) { return p - n; }
int read(int* p, long n) { return p[n]; }
int read_reversed(int* p, long n) { return n[p]; }
int* step(int* p) { return ++p; }
int main() {
  int values[4] = {11, 17, 23, 31};
  int* end = advance(values, 4);
  return *retreat(end, 2) != 23 || read(values, 1) != 17 ||
         read_reversed(values, 3) != 31 || step(values) != values + 1;
}
