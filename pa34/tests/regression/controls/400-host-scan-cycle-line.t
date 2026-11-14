// cycles: 18
// Scans that leave on the data they read, each after a different amount of
// code, in sections of their own and in the unit's section.
volatile unsigned sink;

template<unsigned K> struct Before
{
  static void run()
  {
    sink = K;
    Before<K - 1>::run();
  }
};

template<> struct Before<0>
{
  static void run() {}
};

struct Kinds
{
  unsigned char stop;
  unsigned char last;
  unsigned char kind[256];
};

template<unsigned K>
__attribute__((noinline)) unsigned scan(const unsigned char* text, unsigned size, const Kinds* kinds)
{
  Before<K>::run();
  unsigned at = 0;
  do
  {
    const unsigned char c = text[at];
    if(c == kinds->stop || !(kinds->kind[c] & 1)) break;
    ++at;
  } while(at < size);
  return at;
}

// A longer cycle: five tests of the next element.
template<unsigned K>
__attribute__((noinline)) unsigned scan_word(const unsigned char* text, unsigned size, const Kinds* kinds)
{
  Before<K>::run();
  unsigned at = 0;
  while(at + 1 < size && !(kinds->kind[text[at + 1]] & 2) && text[at + 1] != kinds->stop && text[at + 1] != '"' &&
        text[at + 1] != 0x1e && text[at + 1] != kinds->last)
    ++at;
  return at;
}

__attribute__((noinline)) unsigned scan_first(const unsigned char* text, unsigned size, const Kinds* kinds)
{
  unsigned at = 0;
  do
  {
    if(!(kinds->kind[text[at]] & 1)) break;
    ++at;
  } while(at < size);
  return at;
}

__attribute__((noinline)) unsigned scan_second(const unsigned char* text, unsigned size, const Kinds* kinds)
{
  sink = size;
  unsigned at = 0;
  do
  {
    if(!(kinds->kind[text[at]] & 2)) break;
    ++at;
  } while(at < size);
  return at;
}

int main()
{
  static Kinds kinds;
  kinds.kind[' '] = kinds.kind['\n'] = 3;
  kinds.stop = '\n';
  const unsigned char text[] = "   \n  x  ";
  unsigned bad = 0;
  bad |= scan<0>(text, 9, &kinds) != 3;
  bad |= scan<1>(text, 9, &kinds) != 3;
  bad |= scan<2>(text, 9, &kinds) != 3;
  bad |= scan<3>(text, 9, &kinds) != 3;
  bad |= scan<4>(text, 9, &kinds) != 3;
  bad |= scan<5>(text, 9, &kinds) != 3;
  bad |= scan<6>(text, 9, &kinds) != 3;
  bad |= scan<7>(text, 9, &kinds) != 3;
  bad |= scan<3>(text, 2, &kinds) != 2;
  bad |= scan<5>(text + 4, 5, &kinds) != 2;
  const unsigned char word[] = "abcd efg\"h";
  bad |= scan_word<0>(word, 11, &kinds) != 3;
  bad |= scan_word<1>(word, 11, &kinds) != 3;
  bad |= scan_word<2>(word, 11, &kinds) != 3;
  bad |= scan_word<3>(word, 11, &kinds) != 3;
  bad |= scan_word<4>(word, 11, &kinds) != 3;
  bad |= scan_word<5>(word, 11, &kinds) != 3;
  bad |= scan_word<6>(word, 3, &kinds) != 2;
  bad |= scan_word<7>(word + 5, 6, &kinds) != 2;
  bad |= scan_first(text, 9, &kinds) != 6;
  bad |= scan_first(text + 6, 3, &kinds) != 0;
  bad |= scan_second(text, 5, &kinds) != 5;
  return bad;
}
