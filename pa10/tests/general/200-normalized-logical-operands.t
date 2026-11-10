int hits;
bool between(unsigned x) { return x>=1 && x<12; }
bool outside(unsigned x) { return x<1 || x>=12; }
bool nested(int a,int b,int c) { return (a<2 && b>3) || c==4; }
bool rhs_compare(int x,int y) { ++hits; return x<y; }
bool side_effect(int x,int y) { return x!=0 && rhs_compare(x,y); }
int main() {
 unsigned values[]={0,1,2,11,12,13,0x7fffffffu,0x80000000u,0xffffffffu};
 for(unsigned i=0;i<9;++i) {
  unsigned x=values[i];
  if(between(x)!=(x>=1 && x<12)) { return 1; }
  if(outside(x)!=(x<1 || x>=12)) { return 2; }
 }
 for(int a=-2;a<6;++a) for(int b=-2;b<6;++b) for(int c=-2;c<6;++c) {
  if(nested(a,b,c)!=((a<2 && b>3) || c==4)) { return 3; }
 }
 if(side_effect(0,3) || hits!=0) { return 4; }
 if(!side_effect(1,3) || hits!=1) { return 5; }
 return 0;
}
