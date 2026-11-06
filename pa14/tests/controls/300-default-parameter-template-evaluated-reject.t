// N3485 3.3.4p1 and8.3.6p9: parameter hides outer names and cannot be evaluated in a default. CXX11-039, independently reduced lookup controls.
// must be rejected
int value=1;
template<class T> int f(T value,int n=value){return n;}
int main(){return f(0);}
