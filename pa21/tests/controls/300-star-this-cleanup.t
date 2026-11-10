int copies, destroyed;
int fail(){throw 7;}
struct A {
 int value;
 A(int n):value(n){}
 A(const A &other):value(other.value){++copies;}
 ~A(){++destroyed;}
 int run() {
  try{auto closure=[*this, n=fail()]{return value+n;};(void)closure;return 1;}
  catch(int n){return n==7 && copies==destroyed ? 0:2;}
 }
};
int main(){int result;{A a(4);result=a.run();}return result||destroyed!=copies+1;}
