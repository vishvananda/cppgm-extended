int calls;
int seed() { ++calls; return 9; }
template<class T> struct Box { static thread_local int value; };
template<class T> thread_local int Box<T>::value = seed();
int main() {
  int first = Box<int>::value;
  int second = Box<int>::value;
  return first != 9 || second != 9 || calls != 1;
}
