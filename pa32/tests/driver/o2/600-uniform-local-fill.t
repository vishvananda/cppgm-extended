// Complete fills and their invalidations agree with an independent byte oracle.
#include <cstring>

extern "C" void mutate(unsigned char*, unsigned, unsigned);
template<class T, int B> __attribute__((noinline)) unsigned long long positive(unsigned seed) {
 T data[32]; memset(data,B,sizeof data); return data[seed%32];
}
template<class T, int B> __attribute__((noinline)) unsigned long long negative(unsigned seed, unsigned mode) {
 T data[32]; memset(data,B,sizeof data); unsigned index=seed%32;
 if(mode==1) data[index]=static_cast<T>(seed*129u);
 if(mode==2) mutate(reinterpret_cast<unsigned char*>(data),sizeof data,seed);
 if(mode==3) { void* p=memset(data,seed&255,sizeof data); return static_cast<T*>(p)[index]; }
 if(mode==4) memset(data,13,sizeof(data)/2);
 if(mode==5) { const volatile T* observed=data; return observed[index]; }
 if(mode==6 && (seed&1)) memset(data,seed&255,sizeof data);
 return data[index];
}
extern "C" unsigned long long sample(unsigned seed,unsigned kind,unsigned mode) {
 switch(kind) {
 case 0: return mode==7 ? positive<unsigned char,0>(seed) : negative<unsigned char,0>(seed,mode);
 case 1: return mode==7 ? positive<unsigned char,165>(seed) : negative<unsigned char,165>(seed,mode);
 case 2: return mode==7 ? positive<unsigned char,-1>(seed) : negative<unsigned char,-1>(seed,mode);
 case 3: return mode==7 ? positive<unsigned short,0>(seed) : negative<unsigned short,0>(seed,mode);
 case 4: return mode==7 ? positive<unsigned short,165>(seed) : negative<unsigned short,165>(seed,mode);
 case 5: return mode==7 ? positive<unsigned short,-1>(seed) : negative<unsigned short,-1>(seed,mode);
 case 6: return mode==7 ? positive<unsigned,0>(seed) : negative<unsigned,0>(seed,mode);
 case 7: return mode==7 ? positive<unsigned,165>(seed) : negative<unsigned,165>(seed,mode);
 case 8: return mode==7 ? positive<unsigned,-1>(seed) : negative<unsigned,-1>(seed,mode);
 case 9: return mode==7 ? positive<unsigned long long,0>(seed) : negative<unsigned long long,0>(seed,mode);
 case 10: return mode==7 ? positive<unsigned long long,165>(seed) : negative<unsigned long long,165>(seed,mode);
 case 11: return mode==7 ? positive<unsigned long long,-1>(seed) : negative<unsigned long long,-1>(seed,mode);
 } return 0;
}

#include <cstdio>
extern "C" unsigned long long sample(unsigned,unsigned,unsigned);
static unsigned calls;
extern "C" void mutate(unsigned char* p,unsigned size,unsigned seed) { ++calls; p[(seed>>5)%size]^=static_cast<unsigned char>(seed+17); }
int main() {
 unsigned checks=0;
 const unsigned widths[]={1,2,4,8}, fills[]={0,165,255};
 for(unsigned seed=0;seed<128;++seed) for(unsigned kind=0;kind<12;++kind) for(unsigned mode=0;mode<8;++mode) {
  unsigned width=widths[kind/3], byte=fills[kind%3], index=seed%32;
  unsigned char memory[256]; std::memset(memory,byte,width*32); unsigned expect_calls=0;
  if(mode==1) { unsigned long long value=seed*129u;std::memcpy(memory+index*width,&value,width); }
  if(mode==2) { memory[(seed>>5)%(width*32)]^=static_cast<unsigned char>(seed+17);expect_calls=1; }
  if(mode==3 || (mode==6 && (seed&1))) std::memset(memory,seed&255,width*32);
  if(mode==4) std::memset(memory,13,width*16);
  unsigned long long expected=0; std::memcpy(&expected,memory+index*width,width);calls=0;
  unsigned long long got=sample(seed,kind,mode);
  if(got!=expected || calls!=expect_calls) { std::printf("mismatch %u %u %u %llx %llx\n",seed,kind,mode,got,expected);return 1; }
  ++checks;
 }
 if(checks!=12288)return 2;return 0;
}
