// A selected immutable table has several data-dependent exits. Check the
// empty table, each match, the sentinel and an absent value through all routes.
static const int empty[] = {0};
static const int first[] = {3, 5, 7, 0};
static const int second[] = {2, 4, 6, 8, 10, 12, 0};
static const int third[] = {17, 11, 13, 19, 23, 29, 31, 37, 41, 43, 0};

__attribute__((noinline)) bool contains(unsigned group, int key)
{
  const int* table = 0;
  switch(group) {
    case 0: table = empty; break;
    case 1: table = first; break;
    case 2: table = second; break;
    case 3: table = third; break;
  }
  if(table) {
    unsigned index = 0;
    while(table[index]) {
      if(table[index] == key) return true;
      ++index;
    }
  }
  return false;
}

int main()
{
  const int* tables[] = {empty, first, second, third};
  const unsigned sizes[] = {0, 3, 6, 10};
  for(unsigned group = 0; group != 5; ++group) {
    for(int key = -1; key != 48; ++key) {
      bool expected = false;
      if(group != 4)
        for(unsigned n = 0; n != sizes[group]; ++n)
          expected = expected || tables[group][n] == key;
      if(contains(group, key) != expected) return 1;
    }
  }
  return 0;
}
