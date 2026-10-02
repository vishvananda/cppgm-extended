int calls;int get(){++calls;return 3;}int main(){const auto& x=get();return x!=3||calls!=1;}
