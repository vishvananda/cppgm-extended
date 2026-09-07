struct S {
  S();
};

int main() {
  S* p = new S();
  return p == 0;
}
