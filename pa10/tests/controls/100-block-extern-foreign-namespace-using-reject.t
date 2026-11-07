// expect reject
// N3485 3.5p6: preserve block declaration provenance and canonical namespace entity linkage.
namespace N{extern int n;}int main(){using ::N::n;extern int n;}
