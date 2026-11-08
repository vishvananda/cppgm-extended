// expect exit 0
// The same selected-element rule applies without the variable-template extension.
template<class... Types> struct Count { static const int value=sizeof...(Types); };
template<class... Types> struct Box {
  template<class T> int count() { return Count<T, Types...>::value; }
};
int main() { Box<int,char> box; return box.count<double>() != 3; }
