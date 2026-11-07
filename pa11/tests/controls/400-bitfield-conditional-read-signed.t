// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int f(bool c){S s={-3,-7,3};int n=-9;int a=c?s.a:s.b;int b=c?s.a:n;return a!=(c?-3:-7)||b!=(c?-3:-9);}int main(){return f(true)||f(false);}
