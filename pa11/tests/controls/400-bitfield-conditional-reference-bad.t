// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect reject
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int main(){S s={1,2,3};bool c=false;int&r=(c?s.a:s.b);return r;}
