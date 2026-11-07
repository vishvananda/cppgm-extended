// N3485 5p11/5.16p1,p4: evaluate only selected bit-field glvalue effects; expect exit 0
struct S{signed int a:5;signed int b:6;signed int guard:3;};
int f(bool a,bool b){S s={1,2,3};int outer=0,inner=0,left=0,middle=0,right=0;(++outer,a)?(++left,s.a=5):(++inner,b)?(++middle,s.b=9):(++right,s.guard=2);return outer!=1||inner!=(a?0:1)||left!=(a?1:0)||middle!=(!a&&b?1:0)||right!=(!a&&!b?1:0)||s.a!=(a?5:1)||s.b!=(!a&&b?9:2)||s.guard!=(!a&&!b?2:3);}int main(){return f(true,false)||f(false,true)||f(false,false);}
