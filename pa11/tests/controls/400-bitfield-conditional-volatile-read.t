// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
void observe(bool c,volatile S*p){c?p->a:p->b;}int main(){volatile S s={1,2,3};observe(true,&s);observe(false,&s);return s.a!=1||s.b!=2||s.guard!=3;}
