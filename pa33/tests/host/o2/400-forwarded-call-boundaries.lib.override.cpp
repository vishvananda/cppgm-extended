extern "C" unsigned long long weak_entry(unsigned long long value) {
  return value ^ 0x19734def89844ab1ULL;
}
extern "C" unsigned long long host_apply(
    unsigned long long (*function)(unsigned long long), unsigned long long value) {
  return function(value);
}
