struct Bundle { long a, b, c, d; };

extern "C" {
__attribute__((noinline)) void fill(Bundle *out, long value) {
  out->a = value;
  out->b = value + 1;
  out->c = value + 2;
  out->d = value + 3;
}

__attribute__((noinline)) Bundle construct(const Bundle *input) {
  Bundle out;
  long first = input->a;
  fill(&out, first);
  out.b += input->a;
  out.c += input->a;
  return out;
}

__attribute__((noinline)) long overlap(Bundle *input, Bundle *out) {
  long first = input->a;
  fill(out, 9);
  return first + input->a;
}

__attribute__((noinline)) void nested_fill(Bundle *out, long value) {
  fill(out, value);
}

__attribute__((noinline)) Bundle nested_construct(const Bundle *input) {
  Bundle out;
  long first = input->a;
  nested_fill(&out, first);
  out.d += input->a;
  return out;
}

__attribute__((noinline)) void fill_first(Bundle *out, long value) {
  out->a = value;
}

__attribute__((noinline)) long range(Bundle *input, long value) {
  long first = input->d;
  fill_first(input, value);
  return first + input->d;
}

long global_value;
__attribute__((noinline)) void fill_global(Bundle *out, long value) {
  fill(out, value);
  global_value = value;
}

__attribute__((noinline)) Bundle construct_global(long value) {
  Bundle out;
  long first = global_value;
  fill_global(&out, value);
  out.d += first + global_value;
  return out;
}

__attribute__((noinline)) void fill_two(Bundle *out, long *other, long value) {
  out->a = value;
  *other = value + 1;
}

__attribute__((noinline)) Bundle construct_two(Bundle *input) {
  Bundle out = {};
  long first = input->a;
  fill_two(&out, &input->a, 7);
  out.d = first + input->a;
  return out;
}

__attribute__((noinline)) void fill_dynamic(Bundle *out, unsigned long offset, long value) {
  *reinterpret_cast<long *>(reinterpret_cast<char *>(out) + offset) = value;
}

__attribute__((noinline)) long dynamic_range(Bundle *input, unsigned long offset) {
  long first = input->d;
  fill_dynamic(input, offset, 17);
  return first + input->d;
}

__attribute__((noinline)) void fill_volatile(volatile long *out, long value) {
  *out = value;
}

__attribute__((noinline)) long volatile_control(Bundle *input) {
  long first = input->a;
  fill_volatile(&input->a, 19);
  return first + input->a;
}

__attribute__((noinline)) void fill_recursive(Bundle *out, unsigned n) {
  if (n) fill_recursive(out, n - 1);
  else out->a = 23;
}

__attribute__((noinline)) long recursive_control(Bundle *input) {
  long first = input->a;
  fill_recursive(input, 2);
  return first + input->a;
}

__attribute__((noinline)) Bundle loop_construct(const Bundle *input, unsigned n) {
  Bundle out = {};
  for (unsigned i = 0; i < n; ++i) fill(&out, input->a + i);
  return out;
}

__attribute__((noinline)) Bundle loop_stores(const Bundle *input, unsigned n) {
  Bundle out = {};
  for (unsigned i = 0; i < n; ++i) {
    out.a = input->a;
    out.d += input->a;
  }
  return out;
}
}

int main() {
  Bundle input = {4, 0, 0, 0};
  Bundle built = construct(&input);
  if (built.a != 4 || built.b != 9 || built.c != 10 || built.d != 7) return 1;
  if (overlap(&input, &input) != 13) return 2;
  Bundle other = {0, 0, 0, 0};
  if (overlap(&input, &other) != 18) return 3;
  global_value = 2;
  Bundle global = construct_global(6);
  if (global.a != 6 || global.d != 17 || global_value != 6) return 4;
  Bundle nested = nested_construct(&input);
  if (nested.a != 9 || nested.d != 21) return 5;
  input.d = 11;
  if (range(&input, 5) != 22 || input.a != 5) return 6;
  Bundle two = construct_two(&input);
  if (two.a != 7 || two.d != 13 || input.a != 8) return 7;
  if (dynamic_range(&input, 24) != 28 || input.d != 17) return 8;
  if (dynamic_range(&input, 0) != 34 || input.a != 17) return 9;
  if (volatile_control(&input) != 36 || input.a != 19) return 10;
  if (recursive_control(&input) != 42 || input.a != 23) return 11;
  Bundle loop = loop_construct(&input, 3);
  if (loop.a != 25 || loop.d != 28) return 12;
  Bundle stores = loop_stores(&input, 3);
  if (stores.a != 23 || stores.d != 69) return 13;
  Bundle empty1 = loop_construct(0, 0);
  Bundle empty2 = loop_stores(0, 0);
  if (empty1.a || empty1.d || empty2.a || empty2.d) return 14;
  return 0;
}
