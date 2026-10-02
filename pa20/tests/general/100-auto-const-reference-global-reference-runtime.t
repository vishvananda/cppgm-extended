int calls;struct S{int x;S(int v):x(v){++calls;}};const auto& s=S(3);int main(){return s.x!=3||calls!=1;}
