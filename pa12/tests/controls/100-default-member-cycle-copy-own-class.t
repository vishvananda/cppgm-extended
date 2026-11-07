// CWG1397 forbids defaulted default constructor cycles, not copy/move construction; expect exit 0
struct X; extern X seed; struct X { int i = (X(seed),0); X(int n):i(n){} }; X seed(7); int main(){ return seed.i != 7; }
