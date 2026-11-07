// expect exit 0
unsigned evaluations;
struct Range {
  int values[2];
  int* begin() & { return values; }
  int* end() & { return values + 2; }
};
Range make() { ++evaluations; return Range{{2,3}}; }
template<class T> int sum() {
  int result = 0;
  for (auto& value : make()) { result += value; ++value; }
  return result;
}
int main() { return sum<int>() != 5 || evaluations != 1; }
