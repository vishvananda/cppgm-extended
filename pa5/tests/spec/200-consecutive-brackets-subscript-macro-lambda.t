// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
#define INDEX []{return 0;}()
int a[1]; int main(){ return a[INDEX]; }
