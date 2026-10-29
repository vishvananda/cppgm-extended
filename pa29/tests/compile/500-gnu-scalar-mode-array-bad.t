// The complete declared array is not a scalar machine-mode type.
int wrong[2] __attribute__((mode(DI)));
int main(){return sizeof(wrong);}
