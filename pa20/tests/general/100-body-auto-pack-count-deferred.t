template<class... Types> unsigned count() {
  constexpr auto value = 1 + sizeof...(Types);
  return value;
}
int main() { return count<>() != 1 || count<int, char>() != 3; }
