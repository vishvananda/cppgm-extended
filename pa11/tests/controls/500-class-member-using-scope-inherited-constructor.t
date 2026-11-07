// N3485 7.3.3p8 and adopted CWG1742: class-member using declarations require class scope, except enumerators. expect exit 0
struct B{int n;B(int x):n(x){}};struct D:B{using B::B;};int main(){D d(9);return d.n==9?0:1;}
