extern "C" double scalar_conversions(int a, unsigned b, float c, double d) {
  const signed char narrow = static_cast<signed char>(a);
  return static_cast<double>(narrow) + static_cast<double>(b) + c * d;
}
extern "C" long long wide_conversion(double x, long long y) {
  return static_cast<long long>(x) + y;
}
