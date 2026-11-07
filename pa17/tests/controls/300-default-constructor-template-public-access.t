// expect exit 0
// N3485 11,14.5.2: constructor template specializations retain primary access.
int n; struct D { template<class T=int> D(){++n;} }; struct F { D d; F(){} }; int main(){F f;return n!=1;}
