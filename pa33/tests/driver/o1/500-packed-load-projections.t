struct Sample { unsigned char tag; float value; };
static inline bool accepts(float bound, Sample sample) { return bound >= sample.value; }

extern "C" __attribute__((noinline)) unsigned count_samples(float bound, const Sample* samples, unsigned size)
{
  unsigned count = 0;
  for (unsigned i = 0; i < size; ++i)
    count += accepts(bound, samples[i]);
  return count;
}

extern "C" __attribute__((noinline)) unsigned float_field(const Sample* sample)
{
  Sample copy = *sample;
  unsigned bits;
  __builtin_memcpy(&bits, &copy.value, sizeof bits);
  return bits;
}

int main()
{
  Sample samples[7] = {{1, -3.0f}, {2, 0.0f}, {3, -0.0f}, {4, 1.0f}, {5, 3.0f}, {6, 5.0f}, {7, 7.0f}};
  if (count_samples(3.0f, samples, 7) != 5 || count_samples(-1.0f, samples, 7) != 1)
    return 1;
  const unsigned patterns[] = {0U, 0x80000000U, 0x3f800000U, 0x7fc12345U, 1U, 0xffc23456U};
  for (unsigned i = 0; i < sizeof patterns / sizeof *patterns; ++i) {
    Sample sample = {static_cast<unsigned char>(i), 0.0f};
    __builtin_memcpy(&sample.value, &patterns[i], sizeof sample.value);
    if (float_field(&sample) != patterns[i]) return 2;
  }
  return 0;
}
