// Access to an unrelated specialization is not granted by an inline friend
// definition alone; the trailing return substitution must reject it.
// expect reject
template<class T> class Iter {
 T* pointer;
public:
 Iter(T* p) : pointer(p) {}
 template<class U> friend auto operator-(const Iter& a, const Iter<U>& b)
   -> decltype(a.pointer - b.pointer) { return a.pointer - b.pointer; }
};
int main() {
 int values[4] = {};
 return Iter<const int>(values + 3) - Iter<int>(values);
}
