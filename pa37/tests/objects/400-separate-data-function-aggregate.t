extern int state;
extern "C" int bump(int);
extern "C" int (*address())(int);
struct Pair { int x, y; };
extern "C" Pair adjust(Pair);
extern "C" int observe() {
 Pair a = {4, 5}; Pair b = adjust(a);
 int (*p)(int) = address();
 return bump(2) + p(3) + state + b.x + b.y + a.x;
}
