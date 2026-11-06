// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
int main(){int *p=new int[([]{return 1;}())]; p[0]=0; int r=p[0]; delete[]p; return r;}
