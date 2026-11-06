// N3485 3.3.4p1 and8.3.6p9: parameter hides outer names and cannot be evaluated in a default. CXX11-039, independently reduced lookup controls.
// must be rejected
typedef int Value;
void f(long Value,int n=Value(2));
int main(){return 0;}
