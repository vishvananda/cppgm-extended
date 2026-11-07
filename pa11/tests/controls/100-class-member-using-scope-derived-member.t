// N3485 7.3.3p8 and adopted CWG1742: class-member using declarations require class scope, except enumerators. expect exit 0
struct B{int f(){return 7;}};struct D:B{using B::f;};int main(){D d;return d.f()==7?0:1;}
