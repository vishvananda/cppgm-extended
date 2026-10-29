int calls;
template<class E>struct Work{E value;explicit Work(E v):value(v){}int operator()(int x)const{return value+x;}};
struct Base{using executor_type=int;static int get_executor(){++calls;return 37;}};
template<class T>struct Handler{using first=T;typedef first second;using base1_type=second;
 int immediate(int& function){return(Work<typename base1_type::executor_type>(base1_type::get_executor()))(function);}
};
int main(){Handler<Base> h;int x=5;return h.immediate(x)==42&&calls==1?0:1;}
