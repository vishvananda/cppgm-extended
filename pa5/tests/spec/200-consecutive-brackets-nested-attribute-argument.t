// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
// CWG 2079: nested brackets in an attribute argument remain permitted.
int a [[vendor::attr([[]])]]; int main(){return a;}
