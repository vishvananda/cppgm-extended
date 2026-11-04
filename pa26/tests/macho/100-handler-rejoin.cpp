volatile int limit = 9;

int thrower(int value) {
  if (value > 5) throw value;
  return value;
}

int main() {
  long total = 0;
  for (int value = 0; value < limit; ++value) {
    try { total += thrower(value); }
    catch (int caught) { total += 1000 * caught; }
  }
  return total == 21015 ? 0 : 1;
}
