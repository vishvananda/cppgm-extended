// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int f(bool c){S s={1,2,3};int n=7;int r=((c?s.a:n)+=2);return r!=(c?3:9)||s.a!=(c?3:1)||n!=(c?7:9)||s.b!=2||s.guard!=3;}int main(){return f(true)||f(false);}
