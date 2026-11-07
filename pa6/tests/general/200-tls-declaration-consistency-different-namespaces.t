// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect exit 0
namespace A{extern thread_local int n;thread_local int n=7;}namespace B{extern int n;int n=20;}int main(){return A::n!=7||B::n!=20;}
