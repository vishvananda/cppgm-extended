// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect exit 0
thread_local int n=7;int m=20;int f(){thread_local int m=9;return ++m;}int main(){int n=3;return ++n!=4||::n!=7||f()!=10||f()!=11||::m!=20;}
