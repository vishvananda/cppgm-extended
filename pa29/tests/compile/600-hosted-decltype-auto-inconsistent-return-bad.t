int value;
decltype(auto) choose(bool condition) {
  if (condition) return value;
  return (value);
}
