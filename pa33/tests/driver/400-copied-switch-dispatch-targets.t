// Copied switch dispatches must preserve each case through frame splitting
// and block renumbering, including the zero value selected by an incoming edge.
volatile int observed;
__attribute__((noinline)) int f0(int x) { observed = x+0; return observed; }
__attribute__((noinline)) int f1(int x) { observed = x+1; return observed; }
__attribute__((noinline)) int f2(int x) { observed = x+2; return observed; }
__attribute__((noinline)) int f3(int x) { observed = x+3; return observed; }
__attribute__((noinline)) int f4(int x) { observed = x+4; return observed; }
__attribute__((noinline)) int f5(int x) { observed = x+5; return observed; }
__attribute__((noinline)) int f6(int x) { observed = x+6; return observed; }
__attribute__((noinline)) bool read(int x,long long& value){value=x;return false;}
__attribute__((noinline)) int choose(int x) {long long wide;int v;if(read(x,wide))v=0;else{int n=wide;v=(static_cast<long long>(n)==wide)?n:0;} switch(v){
case -3:return f0(v);
case -2:return f1(v);
case -1:return f2(v);
case 0:return f3(v);
case 1:return f4(v);
case 2:return f5(v);
case 3:return f6(v);
default:__builtin_unreachable();}}
int main(){for(int i=-3;i<=3;++i)if(choose(i)!=i+i+3)return i+5;return 0;}
