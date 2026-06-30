template<class T>
struct box {
  box();
};

template<>
struct box<int> {
  box();
};

template<>
box<int>::box() {}
