#include "300-constexpr-base-entry-export.helper.h"
extern "C" void host_base_entry(CtorView<int>*, const CtorBox*)
  asm("_ZN8CtorViewIiEC2I7CtorBoxEERKT_");
extern "C" int host_check_entries() {
  struct Storage {
    unsigned long before;
    alignas(CtorView<int>) unsigned char bytes[sizeof(CtorView<int>)];
    unsigned long after;
  } storage = {0x123456789abcdefUL, {}, 0xfedcba987654321UL};
  for(int n=-4096;n<4096;++n) {
    CtorBox b{n};
    CtorView<int>* p=reinterpret_cast<CtorView<int>*>(storage.bytes);
    host_base_entry(p,&b);
    if(p->value!=n)return 1;
    if(p->value!=n||complete_entry_value(n)!=n)return 2;
    if(storage.before!=0x123456789abcdefUL||storage.after!=0xfedcba987654321UL)return 3;
  }
  return 0;
}
