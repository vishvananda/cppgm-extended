struct S {int value,id; S(); S(int); S(const S&); S(S&&); S& operator=(const S&); S& operator=(S&&); ~S() noexcept;};
extern void check(int); extern void mark(int);
struct B{S a;~B()noexcept{mark(10000);}};struct E:B{S b;E():b(2){} };
extern "C" void construction_probe(){E original;E copy(original);check(copy.a.value==7 && copy.b.value==2);}

static int progress,limit,live,bad,count,marks,trace[4096];
static int next(){if(++progress==limit)throw progress;trace[count++]=progress;++live;return progress;}
S::S():value(7),id(next()){} S::S(int n):value(n),id(next()){}
S::S(const S& x):value(x.value),id(next()){}
S::S(S&& x):value(x.value),id(next()){x.value=-1;}
S& S::operator=(const S& x){if(++progress==limit)throw progress;value=x.value;return *this;}
S& S::operator=(S&& x){if(++progress==limit)throw progress;value=x.value;x.value=-1;return *this;}
S::~S()noexcept{trace[count++]=-id;--live;}
void check(int n){if(!n)++bad;} void mark(int n){trace[count++]=-n;++marks;}
extern "C" void construction_probe();
int main(){int bound=0;for(limit=0;limit<=bound+1;++limit){progress=live=count=marks=0;int caught=0;
try{construction_probe();}catch(int n){caught=n;}
if(limit==0)bound=progress;
if(live || caught!=(limit>0 && limit<=bound?limit:0))++bad;if(marks!=(limit==1?0:(limit==2 || limit==3?1:2)))++bad;
}return bad?10:0;}
