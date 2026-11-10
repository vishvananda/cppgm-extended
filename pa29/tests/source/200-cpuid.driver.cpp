#include <cstdint>
#include <cstdio>
#include <cpuid.h>
extern "C" void query(int,unsigned*);
int main(){
 const unsigned leaves[]={0u,0x80000000u};
 for(unsigned leaf: leaves) {
  unsigned actual[4]={0,0,0,0},a,b,c,d;
  query(leaf,actual);__cpuid(leaf,a,b,c,d);
  if(actual[0]!=a||actual[1]!=b||actual[2]!=c||actual[3]!=d) {
   std::printf("%x: %x %x %x %x != %x %x %x %x\n",leaf,actual[0],actual[1],actual[2],actual[3],a,b,c,d);return 1;
  }
 }
 std::puts("CPUID: all four output registers passed");
}
