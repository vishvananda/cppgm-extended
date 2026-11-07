// expect exit 0
// N3485 9.8: ordinary data and static member functions remain valid in local/nested classes.
int main() {
  struct Local { int member; static int value() { return 3; } struct Nested { int member; static int value() { return 4; } }; };
  Local object = {5}; Local::Nested nested = {6};
  return object.member + nested.member + Local::value() + Local::Nested::value() != 18;
}
