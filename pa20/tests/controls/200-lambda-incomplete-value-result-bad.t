// expect reject
// A deduced class value return requires a definition, unlike a reference return.
template<class T> struct Incomplete;
int main() {
  auto copy_value = [](const Incomplete<int>& value) { return (value); };
  return 0;
}
