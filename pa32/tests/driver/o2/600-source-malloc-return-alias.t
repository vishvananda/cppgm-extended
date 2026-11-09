extern "C" __attribute__((malloc)) int* fresh(unsigned long);
extern "C" int* ordinary(unsigned long);
extern "C" {
int fresh_load(int* p) {
  int* q = fresh(sizeof(int));
  int before = *p;
  *q = 7;
  return before + *p;
}
int ordinary_load(int* p) {
  int* q = ordinary(sizeof(int));
  int before = *p;
  *q = 7;
  return before + *p;
}
}
