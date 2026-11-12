// lowir.md cold (compared here, in the -O0 driver lane, because PA33 owns the
// attribute): a function states cold=yes when any declaration of it carries
// GNU cold, before the declaration, after its declarator-id or as gnu::cold:
// a later declaration adds it and a definition without it keeps it; a member
// and a lambda-declarator's call operator state it; a function template's
// specializations take the template's and an explicit specialization only its
// own; __has_attribute answers 1 for cold and __cold__.
volatile int sink;
__attribute__((cold)) void report(int code);
void report(int code) { sink = code; }
int later(int x);
int later(int x) __attribute__((__cold__));
int later(int x) { return x + 1; }
[[gnu::cold]] int bracketed(int x) { return x * 2; }
template<class T> T fallback(T x) __attribute__((cold));
template<class T> T fallback(T x) { return x + x; }
template<> int fallback<int>(int x) { return x + 1; }
struct Logger
{
  int count;
  __attribute__((cold)) void fail(int code) { count += code; }
};
int ordinary(int x) { return x + 4; }
#if __has_attribute(cold) && __has_attribute(__cold__)
const int advertised = 1;
#else
const int advertised = 0;
#endif

int main()
{
  Logger logger;
  logger.count = 0;
  auto failed = [&](int x) __attribute__((cold)) { logger.fail(x); };
  if (sink != 0)
    report(1);
  failed(1);
  int total = later(1) + bracketed(1) + static_cast<int>(fallback(2.0)) + fallback(3) + ordinary(0) +
    logger.count + advertised;
  return total == 2 + 2 + 4 + 4 + 4 + 1 + 1 ? 0 : 1;
}
