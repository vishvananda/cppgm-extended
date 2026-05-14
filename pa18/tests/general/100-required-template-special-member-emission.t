int total;

template<typename T>
struct box {
  T value;

  box() : value(1) {}
  box(const box & other) : value(other.value + 2) {}
  ~box() { total = total + value; }
};

int main()
{
  {
    box<int> a;
    box<int> b(a);
    total = total + b.value;
  }
  return total - 7;
}
