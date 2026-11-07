// expect exit 0
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
class A;int read(A&);class A{int data=6;friend int read(A& a){return a.data;}};int main(){A a;return read(a)!=6;}
