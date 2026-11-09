// A short data-dependent cycle shares its entry with a longer continuation.
// Exercise the bounds, early exit and both back edges through every link route.
volatile unsigned trace;

__attribute__((noinline)) unsigned escaped(unsigned sum, unsigned value)
{
  const unsigned next = sum * 33 + value + 256;
  trace = next;
  return next;
}

__attribute__((noinline)) unsigned scan(const unsigned char* data, unsigned size)
{
  unsigned position = 0, sum = 0;
  while(position < size) {
    unsigned value = data[position++];
    if(!value) break;
    if(value == ' ' || value == '\t') continue;
    if(value == '\\') {
      if(position == size) break;
      value = data[position++];
      sum = escaped(sum, value);
      continue;
    }
    sum = sum * 33 + value;
  }
  return sum ^ position;
}

unsigned expected(const unsigned char* data, unsigned size)
{
  unsigned position = 0, sum = 0;
  for(; position < size;) {
    const unsigned value = data[position];
    ++position;
    if(value == 0) break;
    if(value != ' ' && value != '\t') {
      if(value != '\\') sum = 33 * sum + value;
      else if(position < size) sum = 33 * sum + 256 + data[position++];
      else break;
    }
  }
  return position ^ sum;
}

int main()
{
  unsigned char input[] = {' ', '\t', 'x', 'y', '\\', 'z', ' ', 'q', '\\', 0, 'a', 0, 'b'};
  for(unsigned rotation = 0; rotation != 13; ++rotation) {
    for(unsigned size = 0; size <= 13 - rotation; ++size)
      if(scan(input + rotation, size) != expected(input + rotation, size)) return 1;
    input[rotation] ^= 1;
  }
  return 0;
}
