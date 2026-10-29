// An integer mode cannot turn a floating declaration into an integer.
typedef float Wrong __attribute__((mode(DI)));
int main(){return sizeof(Wrong);}
