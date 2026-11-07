// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E { value = 2 }; int E = 7; int main() { enum E object = value; return E - object - 5; }
