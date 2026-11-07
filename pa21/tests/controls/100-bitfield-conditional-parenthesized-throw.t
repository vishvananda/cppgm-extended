// Adopted CWG1550/1560: conditional with parenthesized throw retains bit-field glvalue; expect exit 0
struct S{signed int a:5;signed int b:6;};
int f(bool c){S s={1,2};int selected=0;try{c?(++selected,s.a=5):(throw 7);if(selected!=1||s.a!=5||s.b!=2)return 1;(void)(c?s.b=9:(throw 8));if(s.b!=9)return 2;false?s.a:(throw 11);}catch(int n){return c?(n!=11||selected!=1||s.a!=5||s.b!=9):(n!=7||selected!=0||s.a!=1||s.b!=2);}return 3;}int main(){return f(true)||f(false);}
