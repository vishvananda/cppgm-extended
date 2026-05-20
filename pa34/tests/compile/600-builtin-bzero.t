void clear_bytes(char *p, unsigned long n) {
  __builtin_bzero(p, n);
}

int main() {
  char buf[8];
  clear_bytes(buf, sizeof(buf));
  return 0;
}
