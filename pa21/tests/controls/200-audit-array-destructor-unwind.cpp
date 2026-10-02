// AUDIT-ID: EH-ARRAY-DTOR
// AUDIT-EXPECT: run
int created,drops,bad,next=3;struct G{int id;G():id(++created){}~G()noexcept(false){if(id!=next--)bad=1;++drops;if(id==2)throw 7;}};void f(){G a[3];}int main(){try{f();return 1;}catch(int v){return v!=7||bad||next||drops!=3;}}
