// C++11 reference initialization: N3485 8.5.3.
int main(){const int a[2]={1,2};const int(&r)[2]=static_cast<const int(&&)[2]>(a);return &r[0]!=&a[0]||r[1]!=2;}
