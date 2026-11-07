// expect exit 0
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
int target(int x){return x+2;}int main(){void target(double);{int target(int);class Local{friend int target(int);};return target(4)!=6;}}
