struct State {typedef int entry;};
template<class T> struct Wrapper:T {};
struct Derived:Wrapper<State> {
  int sum() const {int values[]={1,2,3};int result=0;for(const entry &value:values)result+=value;return result;}
};
int main(){Derived d;return d.sum()==6?0:1;}
