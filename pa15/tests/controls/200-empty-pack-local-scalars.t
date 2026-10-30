// N3485 8.5p16,14.5.3: an empty expanded initializer value-initializes; expect exit 0
template<class T,class... A> T make(A... a){T value(a...);return value;}
enum E{seven=7};typedef int(*Function)(int);
int main(){return make<int>()!=0||make<unsigned>()!=0||make<bool>()||make<char>()!=0||make<E>()!=static_cast<E>(0)||make<int*>()!=nullptr||make<Function>()!=nullptr||make<const int>()!=0||make<volatile int>()!=0;}
