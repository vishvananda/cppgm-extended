// N3485 7.1.1p1: ordinary declarations agree; CWG1793 instantiation directives are separate. expect reject
void f(){extern int n;}void g(){extern thread_local int n;}
