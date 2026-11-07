// expect reject
// N3485 11.3p11; adopted CWG138/P1787R6 requires a prior matching innermost-block declaration.
void target();int main(){class Local{friend void target();};}
