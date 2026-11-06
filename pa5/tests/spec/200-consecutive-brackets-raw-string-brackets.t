// N3485 focus: 7.6.1p6 [dcl.attr.grammar]; consecutive square-bracket syntax.
const char *p=R"tag([[])tag"; int main(){return p[0]!='[';}
