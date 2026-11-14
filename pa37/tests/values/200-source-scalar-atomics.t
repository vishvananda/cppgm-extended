#ifndef __wasm_atomics__
#error The extension flag must also reach source preprocessing.
#endif
template<class T> int check(unsigned long long seed) {
  T value=static_cast<T>(seed), first=value;
  T added=static_cast<T>(static_cast<unsigned long long>(first)+3);
  if (__atomic_add_fetch(&value,static_cast<T>(3),__ATOMIC_RELAXED)!=added) return 1;
  if (__atomic_exchange_n(&value,static_cast<T>(12),__ATOMIC_ACQ_REL)!=added) return 2;
  T expected=8;
  if (__atomic_compare_exchange_n(&value,&expected,static_cast<T>(22),false,__ATOMIC_SEQ_CST,__ATOMIC_ACQUIRE)) return 3;
  if (expected!=12 || value!=12) return 4;
  if (!__atomic_compare_exchange_n(&value,&expected,static_cast<T>(22),true,__ATOMIC_ACQ_REL,__ATOMIC_ACQUIRE)) return 5;
  if (value!=22 || expected!=12) return 6;
  __atomic_store_n(&value,static_cast<T>(42),__ATOMIC_RELEASE);
  if (__atomic_load_n(&value,__ATOMIC_ACQUIRE)!=42) return 7;
  if (__sync_val_compare_and_swap(&value,static_cast<T>(41),static_cast<T>(8))!=42 || value!=42) return 8;
  if (__sync_val_compare_and_swap(&value,static_cast<T>(42),static_cast<T>(8))!=42 || value!=8) return 9;
  if (__atomic_fetch_and(&value,static_cast<T>(15),__ATOMIC_RELAXED)!=8 || value!=8) return 12;
  if (__atomic_fetch_or(&value,static_cast<T>(5),__ATOMIC_RELAXED)!=8 || value!=13) return 13;
  if (__atomic_fetch_xor(&value,static_cast<T>(3),__ATOMIC_RELAXED)!=13 || value!=14) return 14;
  if (__atomic_sub_fetch(&value,static_cast<T>(1),__ATOMIC_RELAXED)!=13) return 15;
  return 0;
}
extern "C" int observe(long long seed) {
  int result=check<signed char>(seed);
  if (!result) result=check<unsigned char>(seed);
  if (!result) result=check<short>(seed);
  if (!result) result=check<unsigned short>(seed);
  if (!result) result=check<int>(seed);
  if (!result) result=check<unsigned int>(seed);
  if (!result) result=check<long long>(seed);
  if (!result) result=check<unsigned long long>(seed);
  bool flag=false, expected=false;
  if (!__atomic_compare_exchange_n(&flag,&expected,true,false,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)) return 10;
  if (!flag || expected) return 11;
  int words[4];
  int *pointer=words, *expected_pointer=words+1;
  if (__atomic_exchange_n(&pointer,words+2,__ATOMIC_SEQ_CST)!=words) return 18;
  if (__atomic_compare_exchange_n(&pointer,&expected_pointer,words+3,false,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)) return 19;
  if (expected_pointer!=words+2) return 20;
  if (!__atomic_compare_exchange_n(&pointer,&expected_pointer,words+3,false,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)) return 21;
  if (__atomic_load_n(&pointer,__ATOMIC_ACQUIRE)!=words+3) return 22;
  __atomic_store_n(&pointer,words,__ATOMIC_RELEASE);
  if (pointer!=words) return 23;
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  __atomic_signal_fence(__ATOMIC_SEQ_CST);
  return result ? result : 17;
}
