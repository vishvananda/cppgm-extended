// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int bases;S& get(S&s){++bases;return s;}void discarded(S*p){p->a;}int main(){S s={1,2,3};get(s).a;(s.a);static_cast<S&&>(s).b;discarded(&s);return bases!=1||s.a!=1||s.b!=2||s.guard!=3;}
