// expect exit 0
// N3485 3.5p6: preserve block declaration provenance and canonical namespace entity linkage.
int n=5;int main(){using ::n;extern int n;return n!=5;}
