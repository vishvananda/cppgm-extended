// Omitted static aggregate defaults can depend on earlier members and hold references.
int first_global = 13, other_global = 29;
struct Data {
  int seed = 5;
  int value = seed + 2;
  int* pointer = &first_global;
  int& reference = first_global;
};
static const Data rows[] = {{11}, {}, {17, 19, &other_global, other_global}};
int main() {
 if (rows[0].seed != 11 || rows[0].value != 13 || rows[1].seed != 5 || rows[1].value != 7) return 1;
 if (rows[0].pointer != &first_global || &rows[1].reference != &first_global) return 2;
 if (rows[2].seed != 17 || rows[2].value != 19 || rows[2].pointer != &other_global || &rows[2].reference != &other_global) return 3;
 return 0;
}
