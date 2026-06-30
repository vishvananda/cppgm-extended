template<class T>
struct box {
  box();
};

extern template box<int>::box();

int main()
{
  return 0;
}
