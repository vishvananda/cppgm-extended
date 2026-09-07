struct X { int m : 3; };

int main() {
  X x = {3};
  return x.m != 3;
}
