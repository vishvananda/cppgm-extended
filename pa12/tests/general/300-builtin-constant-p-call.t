int f(int x) {
  return __builtin_constant_p(x);
}

int g() {
  return __builtin_constant_p(3) + __builtin_constant_p(1 + 2);
}
