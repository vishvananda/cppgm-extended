// Empty expanded static initializers zero once; arguments still evaluate at each call; expect exit 0
template<class T,class... A> T& remembered(A... a){static T value(a...);return value;}
int calls;int argument(){++calls;return calls==1?5:9;}
int main(){if(remembered<int>()!=0)return 1;remembered<int>()=7;if(remembered<int>()!=7)return 2;if(remembered<int>(argument())!=5||remembered<int>(argument())!=5||calls!=2)return 3;return 0;}
