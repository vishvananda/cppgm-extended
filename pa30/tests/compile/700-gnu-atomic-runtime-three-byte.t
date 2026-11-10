struct Value { unsigned char bytes[3]; };
int main(){Value value={{1,2,3}},out={{0,0,0}},desired={{4,5,6}};__atomic_load(&value,&out,__ATOMIC_ACQUIRE);if(out.bytes[0]!=1||out.bytes[2]!=3)return 1;__atomic_store(&value,&desired,__ATOMIC_RELEASE);return value.bytes[0]!=4||value.bytes[2]!=6;}
