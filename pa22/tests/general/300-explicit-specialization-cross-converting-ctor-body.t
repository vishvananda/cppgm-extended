template<class T>
class complex;

template<>
class complex<float> {
  float re_;
  float im_;

public:
  complex(float re = 0, float im = 0) : re_(re), im_(im) {}
  explicit complex(const complex<double>& c);
  float real() const { return re_; }
  float imag() const { return im_; }
};

template<>
class complex<double> {
  double re_;
  double im_;

public:
  complex(double re = 0, double im = 0) : re_(re), im_(im) {}
  double real() const { return re_; }
  double imag() const { return im_; }
};

inline complex<float>::complex(const complex<double>& c) : re_(c.real()), im_(c.imag()) {}

int main() {
  complex<double> d;
  complex<float> f(d);
  (void)f;
  return 0;
}
