// CWG2082 andP1825/CWG2549: deleted move selected for own parameter in default. CXX11-039, independently reduced lookup controls.
// must be rejected
struct V{V(){}V(const V&){}V(V&&)=delete;};
void f(V value,int n=sizeof((throw value,0)));
int main(){return 0;}
