int shared = 11;
int zero[4096];
int* pointer = &shared;
__attribute__((noinline)) int stack_alignment() {
  alignas(16) int local[4];
  volatile unsigned long address = reinterpret_cast<unsigned long>(local);
  return address & 15;
}
int initial_alignment = stack_alignment();
int helper() {
  zero[3] = *pointer;
  return zero[3] + initial_alignment + stack_alignment();
}
