struct holder {
  float re;
  float im;

  template<class Tag, int = 0>
  explicit holder(Tag, _Complex float v) : re(__real__ v), im(__imag__ v) {}
};

int main() {
  return 0;
}
