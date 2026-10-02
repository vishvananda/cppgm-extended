int calls;struct S{int x;S(int v):x(v){++calls;}};int main(){const auto& s=S(3);return s.x!=3||calls!=1;}
