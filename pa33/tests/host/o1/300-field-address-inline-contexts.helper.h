struct Fields { unsigned long a,b,c,d,e,f,g,h,i; };
extern "C" unsigned long field_sum(const Fields*);
extern "C" unsigned long read_three(const Fields*,const Fields*,const Fields*);
extern "C" int host_check_fields();
