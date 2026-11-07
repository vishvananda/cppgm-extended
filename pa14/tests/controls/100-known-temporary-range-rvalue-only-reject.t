// expect reject
struct Range {
  int* begin() &&;
  int* end() &&;
};
Range make();
template<class T> void iterate() { for (auto value : make()) (void)value; }
