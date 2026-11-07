// expect exit 0
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
int main(){struct Outer{struct Inner;int read(Inner& x){return x.data;}struct Inner{friend int Outer::read(Inner&);private:int data=7;};};Outer a;Outer::Inner b;return a.read(b)!=7;}
