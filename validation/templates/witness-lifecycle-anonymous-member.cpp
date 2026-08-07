template<class T>
struct Holder {
  struct {
    T value;
  };
};

int main() {
  Holder<int> holder;
  holder.value = 7;
  return holder.value - 7;
}
