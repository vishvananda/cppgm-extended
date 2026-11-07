// N3485 3.3.1/3.4.4/7.2: elaborated enum references preserve ordinary lookup.
// expect exit 0
enum E { value = 2 }; template<class T> int E(T n) { return int(n); } enum E object = value; int main() { return E<int>(5) - object - 3; }
