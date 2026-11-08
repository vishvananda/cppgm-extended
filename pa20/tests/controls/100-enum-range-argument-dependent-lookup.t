// expect exit 0
namespace Values {
 enum Range { all };
 int data[3] = { 2, 4, 8 };
 int* begin(Range) { return data; }
 int* end(Range) { return data + 3; }
}
int main() { int sum = 0; for (int value : Values::all) sum += value; return sum == 14 ? 0 : 1; }
