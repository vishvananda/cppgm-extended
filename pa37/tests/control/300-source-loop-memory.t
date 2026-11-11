volatile int values[4];
int fill(int n) {
  for (int i = 0; i < 4; ++i) values[i] = (i + 1) * n;
  return values[3];
}
extern "C" int source_memory(int n) {
  int total = fill(n);
  for (int i = 0; i < 4; ++i) total += values[i];
  return total;
}
