// CWG2082 andP1825/CWG2549: own parameter remains move eligible in an unevaluated default. CXX11-039, independently reduced lookup controls.
// expect exit 0
struct V{V(){}V(const V&)=delete;V(V&&){}};
void f(V value,int n=sizeof((throw value,0)));
int main(){return 0;}
