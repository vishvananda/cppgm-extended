// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
int main(){ int *p=new int[[]{return 1;}()]; delete[] p; }
