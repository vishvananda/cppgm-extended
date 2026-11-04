int shared = 11;
int zero[4096];
int* pointer = &shared;
int helper() {
  zero[3] = *pointer;
  return zero[3];
}
