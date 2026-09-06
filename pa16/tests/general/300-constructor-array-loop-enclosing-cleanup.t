int constructed = 0;
int destroyed = 0;

struct Element {
  int value;

  Element() : value(constructed) {
    constructed = constructed + 1;
  }

  ~Element() {
    destroyed = destroyed + 1;
  }
};

int build()
{
  Element earlier;
  Element elements[12];
  return constructed - destroyed + elements[11].value;
}

int main()
{
  int during = build();
  return during == 25 && constructed == 13 && destroyed == 13 ? 0 : 1;
}
