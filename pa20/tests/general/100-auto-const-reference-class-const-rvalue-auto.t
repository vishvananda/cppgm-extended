struct S{int x;};const S get(){return S{3};}int main(){auto& s=get();return s.x!=3;}
