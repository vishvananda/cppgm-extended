// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect reject
struct S{static thread_local int n;};int S::n=7;
