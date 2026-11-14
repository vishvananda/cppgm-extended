extern "C" int observe(int n) {
  int cells[8] = {};
  for (int i = 0; i < n; ++i)
    cells[i & 7] += i;
  int result = 0;
  for (int j = 0; j < 8; ++j)
    result += cells[j];
  return result;
}
