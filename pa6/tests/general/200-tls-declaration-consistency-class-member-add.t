// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect reject
struct S{static int n;};thread_local int S::n=7;
