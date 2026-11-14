extern "C" void atomic_load_impl(unsigned size, const void *source, void *result, int order) asm("__atomic_load");
extern "C" void atomic_load_impl(unsigned size, const void *source, void *result, int order) {
  if (size!=3 || order!=5) __builtin_trap();
  const unsigned char *in=static_cast<const unsigned char*>(source);
  unsigned char *out=static_cast<unsigned char*>(result);
  for (unsigned i=0;i<size;++i) out[i]=in[i];
}
