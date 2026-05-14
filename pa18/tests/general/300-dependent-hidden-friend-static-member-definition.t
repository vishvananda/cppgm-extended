template<class T, class D>
struct Iter {
  typedef D difference_type;
  static const difference_type block_size;

  friend Iter operator+(difference_type, const Iter &) {
    return Iter();
  }
};

template<class T, class D>
const D Iter<T, D>::block_size = 0;

int main() {
  Iter<int, long> it;
  Iter<int, long> next = 1 + it;
  (void)next;
  return 0;
}
