// A dependent call result still needs the template introducer.
template<class T> int read() {
  auto object = T::make();
  return object.read<int>();
}
