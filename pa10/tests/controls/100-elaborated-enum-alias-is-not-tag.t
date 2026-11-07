// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect reject
enum Original { value = 2 }; typedef Original E; enum E object = value; int main() { return 0; }
