// expect reject
struct absent {};
int(*value)(int) = &absent::template fun<int>;
int main(){return 0;}
