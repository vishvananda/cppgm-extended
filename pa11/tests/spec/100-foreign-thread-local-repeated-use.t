// Every access initializes an imported TLS object through one shared wrapper.
extern thread_local int imported;
int read() { return imported; }
int main() {
  int* first = &imported;
  int* second = &imported;
  return read() != 37 || first != second || *first != 37;
}
