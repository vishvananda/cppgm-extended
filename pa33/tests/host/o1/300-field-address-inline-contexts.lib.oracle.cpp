#include "300-field-address-inline-contexts.helper.h"
using U=unsigned long;
static U expected(const Fields& p) {
  U first=p.a+p.b+p.c, second=p.d+p.e+p.f, third=p.g+p.h+p.i;
  return first+second+third;
}
extern "C" int host_check_fields() {
  U seed=1;
  for(unsigned k=0;k<8192;++k) {
    seed=seed*6364136223846793005UL+1;
    Fields a={seed,~seed,seed*3,seed*7,seed^17,seed+31,seed*13,seed^71,seed-19};
    Fields b={~seed,seed*5,seed-7,seed*11,seed^97,seed-41,seed*17,seed^121,seed+47};
    if(field_sum(&a)!=expected(a)||field_sum(&b)!=expected(b))return 1;
    if(read_three(&a,&a,&a)!=expected(a))return 2;
    if(read_three(&a,&b,&a)!=expected(b))return 3;
    if(read_three(&a,&b,&b)!=expected(a))return 4;
  }
  return 0;
}
