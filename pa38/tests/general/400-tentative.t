int zero;
int zero;
int initialized;
int initialized = 9;
int initialized;
static int local;
static int local;
const int constant_zero;
int main(void) {
  const int unused;
  return zero || initialized != 9 || local || constant_zero || sizeof(unused) != sizeof(int);
}
