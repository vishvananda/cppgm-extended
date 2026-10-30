// expect reject
struct value { typedef int fun; };
int(*result)(int)=&value::template fun<int>;
int main(){return 0;}
