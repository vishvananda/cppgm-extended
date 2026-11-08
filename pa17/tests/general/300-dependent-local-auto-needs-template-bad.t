// A truly dependent local object still requires the template introducer,
// including in a definition whose body is never instantiated.
template<class T> int read(T& input) {
  const auto& object = input;
  return object.read<int>();
}
