// expect exit 0
class Incomplete;
int invoke(Incomplete* object, int (Incomplete::*method)(int), int value) {
  return (object->*method)(value);
}
int main() { return 0; }
