// expect exit 0
// N3485 9.4.2: namespace and nonlocal nested classes may declare static data members.
struct Outer { static int value; struct Nested { static const int constant = 4; static int value; }; };
int Outer::value = 3; int Outer::Nested::value = 5;
int main() { return Outer::value + Outer::Nested::constant + Outer::Nested::value != 12; }
