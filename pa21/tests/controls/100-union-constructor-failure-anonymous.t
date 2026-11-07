// expect exit 0
// CWG1866 adopted DR: initialized variants are destroyed on construction failure, excluded on destruction failure.
int trace; struct D { D(){trace=trace*10+1;} ~D(){trace=trace*10+2;} }; struct S { int prefix; union { D d; int n; }; S():prefix(3),d(){throw 7;} ~S(){} }; int main(){try{S s;}catch(int){return trace==12?0:1;}return 2;}
