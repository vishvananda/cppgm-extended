typedef unsigned int Word;
typedef unsigned long long Wide;
Wide query_words(Word index) {
  Word low, high;
  asm volatile("xgetbv\n\t" : "=a"(low), "=d"(high) : "c"(index));
  return low | (Wide(high) << 32);
}
Wide query_wide(Wide index) {
  Wide low, high;
  asm volatile("xgetbv" : "=a"(low), "=d"(high) : "c"(index) : "memory");
  return low ^ high;
}
