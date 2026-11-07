// expect exit 0
// N3485 3.5p6: preserve block declaration provenance and canonical namespace entity linkage.
static int n=3;int main(){extern int n;extern int n;n=8;return n!=8 || &n!=&::n;}
