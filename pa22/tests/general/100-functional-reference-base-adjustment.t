struct Left { int left; };
struct Right { int right; };
struct Derived : Left, Right { int own; };
int main() {
  Derived object;
  object.left = 3;
  object.right = 7;
  typedef Right& Reference;
  Right& reference = Reference(object);
  reference.right = 9;
  return &reference != static_cast<Right*>(&object) || object.right != 9 || object.left != 3;
}
