#include "300-base-only-entry-export.helper.h"
extern "C" void host_complete(ExportValue*, const ExportBox*)
  asm("_ZN11ExportValueC1ERK9ExportBox");
extern "C" void host_base(ExportValue*, const ExportBox*)
  asm("_ZN11ExportValueC2ERK9ExportBox");
extern "C" int host_check_base_only_exports() {
  struct Storage {
    unsigned long before;
    alignas(ExportValue) unsigned char bytes[sizeof(ExportValue)];
    unsigned long after;
  } storage = {0x123456789abcdefUL, {}, 0xfedcba987654321UL};
  for(int n=-4096;n<4096;++n) {
    ExportBox b{n};
    ExportValue* p=reinterpret_cast<ExportValue*>(storage.bytes);
    host_complete(p,&b);
    if(p->value!=n)return 1;
    host_base(p,&b);
    if(p->value!=n)return 2;
    if(base_only_value(n)!=n||complete_object_value(n)!=n)return 3;
    if(storage.before!=0x123456789abcdefUL||storage.after!=0xfedcba987654321UL)return 4;
  }
  return 0;
}
