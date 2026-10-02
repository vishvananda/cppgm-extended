// C++11 reference initialization: N3485 8.5.3.
int main(){volatile int a=3;const volatile int& r=a;return &r!=&a||r!=3;}
