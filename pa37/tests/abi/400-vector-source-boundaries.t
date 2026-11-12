#include "400-vector-source-boundaries.h"
extern "C" int observe(int n) {
 volatile char pad[8];
 pad[0] = static_cast<char>(n);
 Ints a = {n, 7, -3, 19};
 Ints b = {3, -2, 5, 1};
 if (alignment(&a) || alignment(&b)) return 1;
 Ints result = combine(a, b);
 for (int i = 0; i != 4; ++i)
  if (result[i] != (((a[i] + b[i]) * (a[i] - b[i])) ^ b[i])) return 2;
 Ints chosen = (a & (a > b)) | (b & ~(a > b));
 for (int i = 0; i != 4; ++i)
  if (chosen[i] != (a[i] > b[i] ? a[i] : b[i])) return 3;
 Ints shuffled = __builtin_shufflevector(result, b, 3, 4, 1, 6);
 if (shuffled[0] != result[3] || shuffled[1] != b[0] || shuffled[2] != result[1] || shuffled[3] != b[2]) return 4;
 Box box = {a};
 Box changed = add_box(box, 11);
 for (int i = 0; i != 4; ++i)
  if (changed.value[i] != a[i] + 11 || box.value[i] != a[i]) return 5;
 Floats x = {1.5f, -2.0f, 4.0f, -0.5f};
 Floats y = {2.0f, 3.0f, -1.0f, 4.0f};
 Floats z = floats(x, y);
 for (int i = 0; i != 4; ++i)
  if (z[i] != (x[i] + y[i]) * y[i]) return 6;
 return pad[0] == static_cast<char>(n) ? 17 : 7;
}
