struct Pair {
  int first;
  int second = 7;
};

Pair make_pair(int a) {
  return Pair{a};
}
