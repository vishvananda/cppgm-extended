double global_source = 7.5;
typedef const int& GlobalReference;
const int& global_reference = GlobalReference(global_source);
int main() {
  typedef const int& Integer;
  typedef double&& Real;
  double source = 7.5;
  const int& integer = Integer(source);
  source = 9.5;
  int number = 3;
  double&& real = Real(number);
  number = 5;
  global_source = 9.5;
  return integer != 7 || real != 3.0 || global_reference != 7;
}
