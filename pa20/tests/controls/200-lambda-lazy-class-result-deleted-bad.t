// expect reject
// Forming the deduced value result must instantiate and check the deleted copy.
template<class T> struct Blocked {
  Blocked(const Blocked&) = delete;
};
int main() {
  auto copy_value = [](const Blocked<int>& value) { return (value); };
  return 0;
}
