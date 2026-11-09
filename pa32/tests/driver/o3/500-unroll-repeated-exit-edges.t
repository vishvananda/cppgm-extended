__attribute__((noinline)) int scan(int selector) {
  for(int i=0;i<4;++i) {
    switch(selector+i) { case 0: case 1: return i; default: break; }
  }
  return 4;
}
int main() { return scan(-1)==1 && scan(0)==0 && scan(1)==0 && scan(2)==4?0:1; }
