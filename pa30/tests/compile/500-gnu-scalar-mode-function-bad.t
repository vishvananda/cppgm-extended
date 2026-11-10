// A declaration's mode cannot silently resize a function's return type.
int wrong() __attribute__((mode(DI)));
int main(){return 0;}
