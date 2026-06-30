template<class T>
struct lazy_box {
  typename T::missing_type *p;
};

typedef lazy_box<int> lazy_alias;

int main()
{
  return 0;
}
