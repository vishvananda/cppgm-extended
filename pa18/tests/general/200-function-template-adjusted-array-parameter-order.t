// N3485 focus: 8.3.5p5 and 14.8.2.4; adjust array parameters before ordering.
template<class T> int choose(T[]) { return 1; }
template<class T> int choose(const T[]) { return 2; }
template<class T> int choose(T) { return 3; }
template<class T> int bounded(T[4]) { return 1; }
template<class T> int bounded(const T[4]) { return 2; }
template<class T> int bounded(T) { return 3; }
template<class T, int N> int reference(T (&)[N]) { return 4; }
template<class T, int N> int reference(const T (&)[N]) { return 5; }
template<class T> int function(T (int)) { return 6; }
template<class T> int function(T) { return 7; }
int identity(int value) { return value; }
int main() {
  char ordinary[9] = {};
  const char fixed[9] = {};
  volatile char changing[9] = {};
  const volatile char qualified[9] = {};
  return choose(ordinary) != 1 || choose(fixed) != 2
      || choose(changing) != 1 || choose(qualified) != 2
      || choose("text") != 2 || choose(9) != 3
      || bounded(ordinary) != 1 || bounded(fixed) != 2
      || reference(ordinary) != 4 || reference(fixed) != 5
      || function(identity) != 6;
}
