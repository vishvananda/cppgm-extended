// CWG2476 forbids placeholder in this template argument; expect reject
template<class T>struct Box{}; Box<auto(*)(int)> b; int main(){return 0;}
