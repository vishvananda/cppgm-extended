// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
struct Left{int operator[](long)const{return 1;}};
struct Right{int operator[](unsigned long)const{return 2;}};
struct Derived:Left,Right{using Left::operator[];using Right::operator[];};
int main(){Derived d;return d[1];}
