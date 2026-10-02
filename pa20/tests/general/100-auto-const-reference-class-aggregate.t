struct S{int x;};int main(){const auto& s=S{3};return s.x!=3;}
