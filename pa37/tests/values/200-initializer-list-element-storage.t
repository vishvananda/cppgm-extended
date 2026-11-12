namespace std {
template<class T> class initializer_list {
 const T* first_;
 __SIZE_TYPE__ count_;
public:
 const T* begin() const { return first_; }
 const T* end() const { return first_ + count_; }
};
}
extern "C" int observe() {
 int sum = 0;
 for (int value : {3, 5, 9}) sum += value;
 return sum;
}
