struct S {int value,id;S();S(int);S(const S&);~S()noexcept;};
extern int step_value();extern void handled(int);extern void handler_fail();extern void check(int);extern void mark(int);
struct H{S first;H():first(1){try{try{step_value();}catch(int n){handled(n);throw;}}catch(long){mark(90000);}}};
extern "C" void construction_probe(){H h;check(h.first.value==1);}

static int progress,limit,live,bad,count,seen,trace[4096];
static int next(){if(++progress==limit)throw progress;trace[count++]=progress;++live;return progress;}
S::S():value(7),id(next()){}S::S(int n):value(n),id(next()){}S::S(const S&x):value(x.value),id(next()){}
S::~S()noexcept{trace[count++]=-id;--live;}
int step_value(){if(++progress==limit)throw progress;return 10;}
void handled(int n){if(n!=limit)++bad;++seen;trace[count++]=-10000;}
void handler_fail(){throw 99;}void check(int n){if(!n)++bad;}void mark(int n){trace[count++]=-n;}
extern "C" void construction_probe();

// Destruction traces agree with independent Clang and GCC runs at O0/O2.
static const unsigned long long expected_trace[]={13200130ULL,0ULL,1728007160ULL,13200130ULL};
static const int expected_count[]={2,0,3,2};
int main(){int bound=0;for(limit=0;limit<=bound+1;++limit){progress=live=count=seen=0;int caught=0;
try{construction_probe();}catch(int n){caught=n;}
if(limit==0)bound=progress;int expected=limit>0 && limit<=bound?(seen?limit:limit):0;
if(live || caught!=expected)++bad;
unsigned long long hash=0;for(int i=0;i<count;++i)hash=hash*131+static_cast<unsigned long long>(trace[i]+100000);
if(limit>=static_cast<int>(sizeof(expected_trace)/sizeof(expected_trace[0])) || hash!=expected_trace[limit] || count!=expected_count[limit])++bad;
}return bad?10:0;}
