// An SSE/SSEUP parameter occupies all sixteen bytes of an XMM spill.
// Reading only its upper half must keep the incoming parameter store.
using Bytes = char __attribute__((vector_size(16)));
__attribute__((noinline)) Bytes interleave_high(Bytes a, Bytes b)
{
  Bytes result{};
  result[0] = a[8];
  result[1] = b[8];
  result[2] = a[15];
  result[3] = b[15];
  return result;
}
int main()
{
  Bytes a{}, b{};
  for (int i = 0; i < 16; ++i)
  {
    a[i] = i;
    b[i] = 63 - i;
  }
  const Bytes result = interleave_high(a, b);
  return result[0] == 8 && result[1] == 55 &&
         result[2] == 15 && result[3] == 48 ? 0 : 1;
}
