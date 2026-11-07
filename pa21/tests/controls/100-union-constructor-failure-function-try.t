// expect exit 0
// CWG1866 adopted DR: initialized variants are destroyed on construction failure, excluded on destruction failure.
int trace; struct D { int n; D(int x=1):n(x){ trace=trace*10+n; } ~D(){ trace=trace*10+n+1; } }; union U { D d; U() try:d(){throw 7;}catch(int){if(trace!=12)throw 8;} ~U(){} }; int main(){try{U u;}catch(int x){return x==7 && trace==12?0:1;}return 2;}
