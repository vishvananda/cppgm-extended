// N3485 3.5/3.7.2: every declaration retains namespace TLS duration; expect exit 0
namespace A{int f(){extern thread_local int n;return ++n;}thread_local int n=7;}namespace B{int f(){extern thread_local int n;return ++n;}thread_local int n=20;}int main(){return A::f()!=8||B::f()!=21||A::n!=8||B::n!=21||&A::n==&B::n;}
