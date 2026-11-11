extern "C" long pointer_difference(int n) {
  int values[4];
  int* begin = values;
  int* end = begin + (n & 3);
  long span = begin - end;
  unsigned long narrow = reinterpret_cast<unsigned long>(begin);
  unsigned long long wide = reinterpret_cast<unsigned long long>(begin);
  int* round = reinterpret_cast<int*>(wide);
  return span + (round == begin && narrow == wide ? 10 : 0);
}
