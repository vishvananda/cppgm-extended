// expect reject
struct value { static int fun; };
int(*result)(int)=&value::template fun<int>;
int main(){return 0;}
