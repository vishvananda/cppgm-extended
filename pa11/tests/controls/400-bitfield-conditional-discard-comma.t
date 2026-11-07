// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int f(bool c){S s={1,2,3};int before=0;(++before,c?s.a=5:s.b=9);return before!=1||s.a!=(c?5:1)||s.b!=(c?2:9)||s.guard!=3;}int main(){return f(true)||f(false);}
