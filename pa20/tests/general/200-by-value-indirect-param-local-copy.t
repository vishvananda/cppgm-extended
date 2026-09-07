struct Box {
  int x;
  Box();
  Box(const Box &);
  Box(Box &&);
  ~Box();
};

Box forward_box(Box box) {
  return box;
}
