template<class T> struct buffer {
  struct member { T value; };
  alignas(__alignof__(member::value)) char data;
};
buffer<long> value;
