// expect reject
struct pair{int a,b;};
template<class T>int body(){T value{1,2,3};return value.a;}
template<class T>auto probe(int)->decltype(body<T>());
template<class>long probe(...);
static_assert(sizeof(probe<pair>(0))==sizeof(int),"body is outside immediate context");
int main(){return body<pair>();}
