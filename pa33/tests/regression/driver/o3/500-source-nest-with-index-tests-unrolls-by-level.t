// A product of two 3x3 matrices whose element accesses test their indexes
// and leave through failed assertions.
__attribute__((noreturn, noinline)) void fail(int line)
{
  throw line;
}

struct Matrix
{
  int a[3][3];
};

inline int element(const Matrix& m, int row, int column)
{
  if (row < 0 || row >= 3)
    fail(1);
  if (column < 0 || column >= 3)
    fail(2);
  return m.a[row][column];
}

inline void set_element(Matrix& m, int row, int column, int value)
{
  if (row < 0 || row >= 3)
    fail(3);
  if (column < 0 || column >= 3)
    fail(4);
  m.a[row][column] = value;
}

__attribute__((noinline)) void product(Matrix& out, const Matrix& x, const Matrix& y)
{
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) {
      int sum = 0;
      for (int k = 0; k < 3; ++k)
        sum += element(x, i, k) * element(y, k, j);
      set_element(out, i, j, sum);
    }
}

Matrix x = {{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}};
Matrix y = {{{0, 1, 0}, {1, 0, 0}, {0, 0, 2}}};

int main()
{
  Matrix out;
  product(out, x, y);
  return out.a[0][0] == 2 && out.a[1][1] == 4 && out.a[2][2] == 18 && out.a[2][0] == 8 ? 0 : 1;
}
