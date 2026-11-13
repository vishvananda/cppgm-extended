// Every eighth pass reads and writes one element of `hashes`: the vector loop
// keeps the eight hashes in two registers of four lanes, and the scalar loop
// does the passes that are left (5 of 13; all of 8 is one round).
static unsigned char data[1000];

__attribute__((noinline)) static unsigned digest(unsigned long n)
{
  unsigned hashes[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  for (unsigned long i = 0; i < n; i++) {
    unsigned& hash = hashes[i % 8];
    hash = ((hash << 5) + hash) + data[i];
    hash = (hash + (hash << 10)) ^ (hash >> 6);
  }
  unsigned all = 0;
  for (int k = 0; k < 8; k++)
    all = all * 31 + hashes[k];
  return all;
}

int main()
{
  for (int i = 0; i < 1000; ++i)
    data[i] = (unsigned char)(i * 7);
  return digest(1000) == 911731218u && digest(13) == 3500121845u && digest(8) == 793687266u ? 0 : 1;
}
