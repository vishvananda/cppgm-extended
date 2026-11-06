// N3485 3.3.4p1 and8.3.6p9: parameter hides outer names and cannot be evaluated in a default. CXX11-039, independently reduced lookup controls.
// must be rejected
struct S{static int value;int f(int value,int n=value);};
int main(){return 0;}
