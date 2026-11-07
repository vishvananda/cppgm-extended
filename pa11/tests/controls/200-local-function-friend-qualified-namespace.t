// expect exit 0
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
int target(){return 9;}int main(){class Local{friend int ::target();};return target()!=9;}
