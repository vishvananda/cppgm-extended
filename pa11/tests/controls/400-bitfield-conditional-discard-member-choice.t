// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int bases;S& get(S&s){++bases;return s;}int f(bool c){S x={1,2,3},y={4,5,3};int left=0,right=0;bases=0;c?(++left,get(x).a):(++right,get(y).b);return bases!=1||left!=(c?1:0)||right!=(c?0:1)||x.a!=1||y.b!=5;}int main(){return f(true)||f(false);}
