int calls;struct S{int x;S(int v):x(v){++calls;}};const S& get(){static const auto& s=S(3);return s;}int main(){const S& s=get();return s.x!=3||calls!=1||&s!=&get();}
