template<class T>
struct Box {
  int out();
  int in() { return 4; }
};

template<class T>
int Box<T>::out()
{
  return 9;
}

template struct Box<int>;
