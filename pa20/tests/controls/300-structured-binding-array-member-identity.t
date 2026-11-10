struct Pair { int first; int second; };
int main() { int values[2]={2,3}; auto& [a,b]=values; a=5; Pair pair{7,11}; auto [x,y]=pair; return values[0]==5 && b==3 && x+y==18?0:1; }
