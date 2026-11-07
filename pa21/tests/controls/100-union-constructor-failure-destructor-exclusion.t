// expect exit 0
// CWG1866 adopted DR: initialized variants are destroyed on construction failure, excluded on destruction failure.
int trace; struct D { int n; D(int x=1):n(x){ trace=trace*10+n; } ~D(){ trace=trace*10+n+1; } }; union U { D d; U():d(){} ~U() noexcept(false){throw 7;} }; int main(){try{U u;}catch(int){return trace==1?0:1;}return 2;}
