int copies, destroyed;
struct A {
 int x;
 A(int value):x(value){}
 A(const A &other):x(other.x){++copies;}
 ~A(){++destroyed;}
 int run(){auto copy=[*this]() mutable {return ++x;}; auto ptr=[this]{return x;}; x=20; return copy()==8 && ptr()==20 ? 0:1;}
};
int main(){int result;{A a(7);result=a.run();if(copies!=1||destroyed!=1||a.x!=20)return 2;}return result||destroyed!=2;}
