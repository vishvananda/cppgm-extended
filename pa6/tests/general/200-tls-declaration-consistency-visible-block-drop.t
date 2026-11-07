// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect reject
thread_local int n=7;void f(){extern int n;}
