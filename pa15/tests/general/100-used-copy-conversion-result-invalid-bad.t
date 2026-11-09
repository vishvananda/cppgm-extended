struct Other {};
template<class T>struct Invalid { static_assert(sizeof(T)==0,"must not instantiate"); };
struct Box {
 Box() {}
 Box(const Box&) {}
 Box(Other) {}
 operator Invalid<int>()const;
};
int main() { const Box first; Box second(first); }
