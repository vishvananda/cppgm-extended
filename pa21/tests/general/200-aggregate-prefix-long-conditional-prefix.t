struct S { int id; S(); S(int); S(const S&); ~S() noexcept; };
static int progress, limit, live;
static unsigned trace_digest;
static int next()
{
  if (++progress == limit) throw progress;
  trace_digest = trace_digest * 33u + progress;
  ++live;
  return progress;
}
S::S() : id(next()) {}
S::S(int) : id(next()) {}
S::S(const S&) : id(next()) {}
S::~S() noexcept
{
  trace_digest = trace_digest * 33u - id;
  --live;
}
int step_value() { if (++progress == limit) throw progress; return 10; }
int read(const S&) { return step_value(); }
extern int step_value(); extern int read(const S&); extern int choice();
struct E { S a; int n[12]; S b; int last; };
extern "C" void construction_probe(){S x(1),y(2); E e={x,{choice()?read(S(0)):read(S(100)),choice()?read(S(1)):read(S(101)),choice()?read(S(2)):read(S(102)),choice()?read(S(3)):read(S(103)),choice()?read(S(4)):read(S(104)),choice()?read(S(5)):read(S(105)),choice()?read(S(6)):read(S(106)),choice()?read(S(7)):read(S(107)),choice()?read(S(8)):read(S(108)),choice()?read(S(9)):read(S(109)),choice()?read(S(10)):read(S(110)),choice()?read(S(11)):read(S(111))},y,step_value()};}
extern "C" int maxstep(){return 29;}

int choice() { return limit & 1; }

static const unsigned expected[] = {
  1378408384u, 0u, 32u, 38048u, 41611712u, 2373708736u, 2373708736u, 4081190656u, 4081190656u, 4160949248u, 4160949248u, 1679950656u, 1679950656u, 149239104u, 149239104u, 2141989504u, 2141989504u, 3953503104u, 3953503104u, 101040320u, 101040320u, 3166417600u, 3166417600u, 2879916544u, 2879916544u, 3092418304u, 3092418304u, 404769344u, 404769344u, 151618624u
};
int main()
{
  for (limit = 0; limit <= maxstep(); ++limit)
  {
    progress = live = 0;
    trace_digest = 0;
    int caught = 0;
    try { construction_probe(); } catch (int n) { caught = n; }
    if (live || caught != (limit == 0 ? 0 : limit)) return 1;
    if (trace_digest != expected[limit]) return 2;
  }
  return 0;
}
