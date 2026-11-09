struct Result { unsigned long a, b, c, d; };
extern "C" __attribute__((noinline)) Result pack(const Result& input) {
  Result out;
  out.a = input.a;
  out.b = input.b;
  out.c = input.c;
  out.d = input.d;
  return out;
}
int main() {
  for (unsigned long i=0; i!=1024; ++i) {
    Result input = {i, i*3+1, i*7+2, i*0x100000001UL+3};
    Result result = pack(input);
    if (result.a!=input.a || result.b!=input.b || result.c!=input.c || result.d!=input.d) return 1;
  }
  return 0;
}
