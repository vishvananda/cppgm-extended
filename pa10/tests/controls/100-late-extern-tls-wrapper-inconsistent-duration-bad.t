// N3485 3.5/3.7.2: every declaration retains namespace TLS duration; expect reject
extern thread_local int n;int n=7;int main(){return n;}
