typedef unsigned long size_t;

template<size_t Words, size_t Size>
struct storage {
  storage();
  storage &operator&=(const storage &rhs);
};

template<size_t Words, size_t Size>
storage<Words, Size>::storage()
{
}

template<size_t Words, size_t Size>
storage<Words, Size> &storage<Words, Size>::operator&=(const storage &rhs)
{
  (void)rhs;
  return *this;
}

template<size_t Size>
struct bits
    : storage<Size == 0 ? 0 : (Size - 1) / (sizeof(size_t) * 8) + 1,
              Size> {
  typedef storage<Size == 0 ? 0 : (Size - 1) / (sizeof(size_t) * 8) + 1,
                  Size> base;
  bits &operator&=(const bits &rhs);
};

template<size_t Size>
bits<Size> &bits<Size>::operator&=(const bits &rhs)
{
  base::operator&=(rhs);
  return *this;
}

int main()
{
  bits<1> value;
  value &= value;
  return 0;
}
