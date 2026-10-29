// The native pointer mode must retain the target pointer width.
int* wrong __attribute__((mode(QI)));
int main(){return 0;}
