// AUDIT-ID: TMPL-FTRY
// AUDIT-EXPECT: run
int drops,trace;struct G{int id;G(int n):id(n){}~G()noexcept(false){++drops;trace=trace*10+id;if(id==2)throw 7;}};template<class T>int f(T)try{G a(1),b(2);return 8;}catch(int v){return v;}int main(){return f(0)!=7||drops!=2||trace!=21;}
