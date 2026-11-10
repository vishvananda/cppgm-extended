unsigned query(unsigned index) {
  unsigned low, high;
  asm volatile("xgetbv" : "=a"(low), "=b"(high) : "c"(index));
  return low ^ high;
}
