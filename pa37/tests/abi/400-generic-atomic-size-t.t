struct Three { unsigned char a,b,c; };
extern "C" int observe() {
  Three source={1,2,3}, result={0,0,0};
  __atomic_load(&source,&result,__ATOMIC_SEQ_CST);
  return result.a==1 && result.b==2 && result.c==3 ? 17 : 0;
}
