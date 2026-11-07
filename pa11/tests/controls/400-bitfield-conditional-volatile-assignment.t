// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
void apply(bool c,volatile S*p){c?p->a=5:p->b=9;}int main(){volatile S s={1,2,3};apply(true,&s);if(s.a!=5||s.b!=2||s.guard!=3)return 1;apply(false,&s);return s.a!=5||s.b!=9||s.guard!=3;}
