// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
int a[1]; int main(){int x=0; return a[([x]{return x;}())]; }
