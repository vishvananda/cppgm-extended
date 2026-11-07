// expect exit 0
// N3485 3.5p6: preserve block declaration provenance and canonical namespace entity linkage.
int n=4;int main(){extern int n;extern int n;n=9;return n!=9;}
