// Optimizer-created memory calls and value-initialized array new use the
// target's size_t boundary, including a wasm32 pointer induction's byte count.
__attribute__((noinline)) void fill(char* first, char* last, char value) {
 while (first != last) *first++ = value;
}
__attribute__((noinline)) void copy(char* output, const char* first, const char* last) {
 while (first != last) *output++ = *first++;
}
__attribute__((noinline)) void fill_words(unsigned* first, unsigned* last, unsigned value) {
 while (first != last) *first++ = value;
}
__attribute__((noinline)) int zeros(unsigned count) {
 int* values = new int[count]();
 int sum = 0;
 for (unsigned i = 0; i != count; ++i) sum |= values[i];
 delete[] values;
 return sum;
}
__attribute__((noinline)) void builtin_copy(char* output, const char* input, unsigned count) {
 __builtin_memmove(output, input, count);
}
int main() {
 volatile unsigned amount = 512;
 unsigned count = amount;
 char source[1024], destination[1024];
 unsigned words[512];
 fill(source, source + count, 37);
 copy(destination, source, source + count);
 builtin_copy(destination, source, count);
 fill_words(words, words + count, 0x12121212u);
 for (unsigned i = 0; i != count; ++i)
  if (source[i] != 37 || destination[i] != 37 || words[i] != 0x12121212u) return 1;
 return zeros(count);
}
