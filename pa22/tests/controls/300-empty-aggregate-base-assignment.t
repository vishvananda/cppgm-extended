struct Base { int first; };
struct Other { int second; };
struct Derived : Base, Other { int third; };
int main() {
  Derived value{{7}, {8}, 9};
  value = {};
  if (value.first || value.second || value.third) return 1;
  value = {11};
  return value.first == 11 && value.second == 0 && value.third == 0 ? 0 : 2;
}
