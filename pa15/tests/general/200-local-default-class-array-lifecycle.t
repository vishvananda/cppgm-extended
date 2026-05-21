int constructed = 0;
int destroyed = 0;

struct Element {
  Element() { constructed = constructed + 1; }
  ~Element() { destroyed = destroyed + 1; }
};

int observed()
{
  return constructed * 10 + destroyed;
}

int main()
{
  {
    Element elements[3];
    if(observed() != 30) {
      return 1;
    }
  }
  return observed() == 33 ? 0 : 2;
}
