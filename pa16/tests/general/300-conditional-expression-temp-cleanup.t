int destroyed = 0;

struct Box {
  Box() {}
  Box(const Box&) {}
  ~Box() { destroyed = destroyed + 1; }
  int* get() const { return 0; }
};

Box keep(const Box& b) { return b; }

int main()
{
  Box box;
  int flag = 0;
  int * q = flag ? keep(box).get() : (int*)0;
  return q == 0 && destroyed == 0 ? 0 : 1;
}
