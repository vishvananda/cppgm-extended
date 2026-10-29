// AUDIT: member-only operator ranking uses the lookup class; the selected call retains its declaring base.
struct Object{int operator[](int)const{return 1;}int operator[](int)volatile{return 2;}};
int main(){Object o;return o[1];}
