struct Path {
  int value;
  template<class T> T read() const { return static_cast<T>(value); }
};
Path global{9};
int factories;
Path make_path() { ++factories; return {7}; }
Path* pointer() { return &global; }
Path& reference() { return global; }
template<class Unused> int exercise() {
  auto const local = make_path();
  decltype(make_path()) named{5};
  auto copy = local;
  auto ptr = pointer();
  auto& ref = reference();
  auto direct = Path();
  auto braces = Path{};
  using Alias = decltype(make_path());
  Alias alias{3};
  decltype(reference()) declared_ref = global;
  return local.read<int>() + named.read<int>() + copy.read<int>() +
      ptr->read<int>() + ref.read<int>() + direct.read<int>() + braces.read<int>() +
      alias.read<int>() + declared_ref.read<int>();
}
struct Owner {
  Path path;
  template<class Unused> int get() const {
    auto& local = this->path;
    return local.read<int>();
  }
};
int main() {
  int result = exercise<int>() + exercise<char>();
  const Owner owner{{4}};
  return result != 98 || factories != 2 || owner.get<int>() != 4;
}
