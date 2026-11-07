// expect exit 0
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
int target(){return 5;}int main(){int target();struct Outer{struct Inner{friend int target();};};return target()!=5;}
