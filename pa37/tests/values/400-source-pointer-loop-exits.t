extern "C" int* advance(int* p, int n) {
  for (int i = 0; i < n; ++i)
    p += 2;
  return p;
}
extern "C" int observe(int n) {
  int values[128];
  return static_cast<int>(advance(values, n) - values);
}
