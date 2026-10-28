template<class A, class B> int cross(const A[], B[]) { return 1; }
template<class A, class B> int cross(A[], const B[]) { return 2; }
int main(){ const char a[3]={}; return cross(a,a); }
