// The inline friend template keeps its lexical class access in a substituted
// trailing return type, including another specialization's granted friendship.
// expect exit 0
template<class T> class Iter {
 T* pointer;
 template<class U> friend class Iter;
public:
 Iter(T* p) : pointer(p) {}
 template<class U> friend auto operator-(const Iter& a, const Iter<U>& b)
   -> decltype(a.pointer - b.pointer) { return a.pointer - b.pointer; }
};
int main() {
 int values[4] = {};
 return Iter<const int>(values + 3) - Iter<int>(values) == 3 ? 0 : 1;
}
