struct Input { unsigned long bits; };
struct Result { unsigned long a, b, c, d; };
extern "C" {
__attribute__((noinline)) Result unpack(const Input& input) {
  Result out = {};
  out.a = input.bits & 1;
  out.b = (input.bits >> 1) & 1;
  out.c = (input.bits >> 2) & 1;
  out.d = (input.bits >> 3) & 1;
  return out;
}
__attribute__((noinline)) void ordinary(Result* out, const unsigned long* input) {
  out->a = *input & 1;
  out->b = (*input >> 1) & 1;
  out->c = (*input >> 2) & 1;
  out->d = (*input >> 3) & 1;
}
}
int main() {
  for (unsigned long i = 0; i != 1024; ++i) {
    Input input = {i * 0x100000001UL};
    Result result = unpack(input);
    if (result.a != (input.bits & 1) || result.b != ((input.bits >> 1) & 1)) return 1;
    if (result.c != ((input.bits >> 2) & 1) || result.d != ((input.bits >> 3) & 1)) return 2;
    Result alias = {input.bits, 0, 0, 0};
    ordinary(&alias, &alias.a);
    if (alias.a != (input.bits & 1) || alias.b != 0 || alias.c != 0 || alias.d != 0) return 3;
  }
  return 0;
}
