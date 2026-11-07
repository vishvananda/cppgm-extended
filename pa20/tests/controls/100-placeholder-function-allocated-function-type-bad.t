// 5.3.4 allocated type must be object type; expect reject
int target(int){return 0;} int main(){auto p=new (auto (int))(target);}
