// Guards constrain full values; wrapping casts, joins and calls still matter.
typedef unsigned long long U;
volatile U source;
volatile unsigned calls;
U digest=0xcbf29ce484222325ull;
void record(U value) { digest=(digest^value)*0x100000001b3ull; }

template<unsigned Bias,unsigned Limit>
__attribute__((noinline)) U byte_range(U value)
{
  long full=static_cast<unsigned char>(value)-static_cast<int>(Bias);
  unsigned char narrow=static_cast<unsigned char>(full);
  if(narrow<=Limit)return static_cast<U>(narrow)+1000;
  return static_cast<U>(full)^0x1234567800000000ull;
}
__attribute__((noinline)) U unknown(U value)
{
  unsigned char narrow=static_cast<unsigned char>(value);
  if(narrow<=9)return static_cast<U>(narrow);
  return value;
}
__attribute__((noinline)) U opaque(U value) { ++calls;return value; }
__attribute__((noinline)) U joined(U value,bool first)
{
  long full=first?static_cast<unsigned char>(value)-17:static_cast<long>(opaque(value));
  unsigned char narrow=static_cast<unsigned char>(full);
  if(narrow<=7)return static_cast<U>(narrow);
  return full;
}
template<unsigned Bias,unsigned Limit>
__attribute__((noinline)) U word_range(unsigned value)
{
  long full=static_cast<unsigned short>(value)-static_cast<int>(Bias);
  unsigned short narrow=static_cast<unsigned short>(full);
  return narrow<=Limit?static_cast<U>(narrow):static_cast<U>(full);
}
__attribute__((noinline)) U digits(const unsigned char* data,unsigned size)
{
  U value=0;
  for(unsigned i=0;i<size;++i) {
    unsigned char digit=data[i]-17;
    if(digit>7)break;
    value=value*10+digit;
  }
  return value;
}
int main()
{
  for(unsigned i=0;i<256;++i) {
    source=0x1234567800000000ull+i;
    U value=source;
    record(byte_range<0,7>(value));record(byte_range<1,9>(value));
    record(byte_range<17,7>(value));record(byte_range<48,9>(value));
    record(byte_range<128,127>(value));record(byte_range<255,0>(value));
    record(byte_range<256,7>(value));record(byte_range<511,9>(value));
    record(unknown(value));record(joined(value,true));record(joined(value,false));
    unsigned char text[8];
    for(unsigned j=0;j<8;++j)text[j]=static_cast<unsigned char>(i+j);
    for(unsigned n=0;n<9;++n)record(digits(text,n));
  }
  for(unsigned i=0;i<65536;++i) {
    record(word_range<257,31>(i));record(word_range<65536,31>(i));
    record(word_range<65535,0>(i));
  }
  if(calls!=256)return 1;
  // Independently evaluated over all byte/word inputs with 64-bit wraparound.
  return digest==0xfea1447d24ce6619ull?0:2;
}
