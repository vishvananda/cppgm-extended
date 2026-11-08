// expect reject
// An actually used member still cannot use a whole outer pack as one argument.
template<class... Types> struct Count { static const int value=sizeof...(Types); };
template<class... Types> struct Box {
  template<class T> int count() { return Count<T, Types>::value; }
};
int main() { Box<int> box; return box.count<double>(); }
