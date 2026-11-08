struct A {int value;
 int run(){auto outer=[this]{return [*this]() mutable {return ++value;};};auto copied=outer();value=20;return copied()==8 && value==20 ? 0:1;}
 int defaults(){auto copy=[=,*this]{return value;};auto refdefault=[&,*this]{return value;};return copy()+refdefault();}
};
int main(){A a{7};return a.defaults()==14 ? a.run():2;}
