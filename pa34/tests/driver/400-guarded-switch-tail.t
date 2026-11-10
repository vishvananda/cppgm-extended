// A packet dispatcher preserves guarded reads, wide tags, calls and cleanup.
extern "C" unsigned opened,closed;
extern "C" unsigned observer(unsigned*,unsigned);
struct Guard { Guard(){++opened;} ~Guard(){++closed;} };
extern "C" __attribute__((noinline)) unsigned process(unsigned* state, const volatile unsigned long long* tags,
                            unsigned n, unsigned flags, unsigned denominator) {
 Guard g; unsigned sum=19;
 for(unsigned i=0;i<n;++i) {
  unsigned v=(i+3)*17u;
  if(flags&1) v+=observer(state+2,i);
  const unsigned long long tag=tags[i];
  if(tag>5) {
   state[3]^=static_cast<unsigned>(tag)+v;
   sum+=(state[i&3]^v)+13/denominator;
   if(flags&2) sum+=observer(state+3,i);
   continue;
  }
  switch(tag) {
   case 0:state[0]+=v;break;
   case 1:state[1]^=state[0]<<(i&7);break;
   case 2:state[2]+=state[1]*3u;break;
   case 3:state[0]^=state[2]>>((i&7)+1);break;
   case 4:state[3]=(state[3]+state[0])^(v|1u);break;
   case 5:{unsigned before=state[0];state[0]=state[1];state[1]=before;break;}
   default:__builtin_unreachable();
  }
  sum+=(state[i&3]^v)+13/denominator;
  if(flags&2) sum+=observer(state+3,i);
 }
 return sum;
}

extern "C" { unsigned opened=0,closed=0; }
static unsigned calls; static bool throwing;
extern "C" __attribute__((noinline)) unsigned observer(unsigned* p,unsigned i) { ++calls;*p=(*p*33u)^(i+17);if(throwing&&i==3)throw 29;return *p; }
extern "C" unsigned process(unsigned*,const volatile unsigned long long*,unsigned,unsigned,unsigned);
static unsigned reference(unsigned* s,const unsigned long long* tags,unsigned n,unsigned flags,unsigned denominator) {
 ++opened;unsigned sum=19;
 try { for(unsigned i=0;i<n;++i) {
  unsigned v=(i+3)*17u;if(flags&1) v+=observer(s+2,i);unsigned long long t=tags[i];
  if(t==0)s[0]+=v;else if(t==1)s[1]^=s[0]<<(i%8);else if(t==2)s[2]+=s[1]*3u;
  else if(t==3)s[0]^=s[2]>>((i%8)+1);else if(t==4)s[3]=(s[3]+s[0])^(v|1u);
  else if(t==5){unsigned v=s[0];s[0]=s[1];s[1]=v;}else s[3]^=static_cast<unsigned>(t)+v;
  sum+=(s[i%4]^v)+13/denominator;if(flags&2)sum+=observer(s+3,i);
 }} catch(...) {++closed;throw;} ++closed;return sum;
}
int main(){
 const unsigned long long special[]={0,1,2,3,4,5,6,7,0x100000000ull,0xffffffffffffffffull};
 for(unsigned seed=0;seed<128;++seed)for(unsigned pattern=0;pattern<4;++pattern)for(unsigned flags=0;flags<4;++flags) {
  unsigned n=seed%65, denominator=n?1+seed%13:0;unsigned long long tags[64];unsigned random=seed*17+13;
  for(unsigned i=0;i<n;++i){random=random*1664525u+1013904223u;tags[i]=special[(pattern==0?i:pattern==1?seed:pattern==2?random:(i/3+seed))%10];}
  unsigned expected[4]={seed,~seed,seed*37,19},actual[4]={seed,~seed,seed*37,19};
  throwing=seed&1;opened=closed=calls=0;unsigned answer=0;bool et=false;
  try{answer=reference(n?expected:0,n?tags:0,n,flags,denominator);}catch(int e){if(e!=29)return 1;et=true;}
  unsigned eo=opened,ec=closed,eb=calls;opened=closed=calls=0;unsigned got=0;bool at=false;
  try{got=process(n?actual:0,n?tags:0,n,flags,denominator);}catch(int e){if(e!=29)return 2;at=true;}
  if(at!=et||got!=answer||opened!=eo||closed!=ec||calls!=eb){return 3;}
  for(unsigned j=0;j<4;++j)if(actual[j]!=expected[j]){return 4;}
 }
 return 0;
}
