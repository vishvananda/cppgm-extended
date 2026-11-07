// expect reject
// N3485 3.5p6: preserve block declaration provenance and canonical namespace entity linkage.
int main(){static int n=0;extern int n;}
