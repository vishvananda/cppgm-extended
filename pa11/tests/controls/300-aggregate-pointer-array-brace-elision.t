// N3485 8.5.1/8.5.2: a string initializes a pointer element or a character array.
// expect exit 0
struct Pointers { const char* values[3]; };
struct Characters { char values[4]; int end; };
Pointers global = {"one", "two", "three"};
Characters characters = {"abc", 7};
int main() {
  Pointers local = {"alpha", "beta", "gamma"};
  Pointers nested = {{"red", "green", "blue"}};
  return global.values[1][0] != 't' || local.values[2][0] != 'g' ||
         nested.values[0][0] != 'r' || characters.values[2] != 'c' || characters.end != 7;
}
