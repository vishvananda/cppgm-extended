// C++11 parameter declaration has no deduced placeholder; expect reject
void f(auto (*)(int)); int main(){return 0;}
