// C++11 reference initialization: N3485 8.5.3.
int f(int(&)[2]){return 1;}int f(const int(&)[2]){return 2;}int main(){int a[2]={1,2};const int b[2]={3,4};return f(a)!=1||f(b)!=2;}
