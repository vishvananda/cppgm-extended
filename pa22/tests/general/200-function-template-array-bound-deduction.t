template<class T, int N>
int array_size(T (&)[N]) {
  return N;
}

int main() {
  int a[3] = {0, 1, 2};
  return array_size(a) - 3;
}
