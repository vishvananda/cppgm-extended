struct Item { int value; struct Item *next; };
int Item = 17;
enum Color { red, blue };
int Color = 9;
struct Bits { unsigned int value : 3; signed int negative : 3; };
typedef struct Alias { int value; } Alias;
int main(void) {
  struct Item second = { 7, 0 }, first = { 3, &second };
  enum Color color = blue;
  Alias alias = { 5 };
  struct Bits bits = { 0, 0 };
  int assigned = (bits.value = 11), stepped = ++bits.value;
  int signed_value = (bits.negative = -2);
  return !(Item == 17 && Color == 9 && color == 1 &&
           first.next->value == 7 && alias.value == 5 &&
           assigned == 3 && stepped == 4 && bits.value == 4 && signed_value == -2);
}
