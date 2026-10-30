int calls;
int next() { ++calls; return 7; }
int shared() { static int value = next(); return value; }
int local() { static thread_local int value = next(); return value; }
int main() {
  if (shared() != 7 || shared() != 7 || calls != 1) return 1;
  if (local() != 7 || local() != 7 || calls != 2) return 2;
  return 0;
}
