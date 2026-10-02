// C++11 reference initialization: N3485 8.5.3.
int main(){volatile int a[2]={1,2};const volatile int(&r)[2]=a;return &r[0]!=&a[0]||r[1]!=2;}
