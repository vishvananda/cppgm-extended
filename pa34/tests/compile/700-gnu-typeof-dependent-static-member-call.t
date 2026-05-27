template<long N>
struct long_ {
  static const long value = N;
};

template<class T>
struct type_wrapper {
  typedef T type;
};

template<class T>
struct wrapped_type;

template<class T>
struct wrapped_type<type_wrapper<T> > {
  typedef T type;
};

struct vector0 {
  typedef long_<0> lower_bound_;
  typedef lower_bound_ upper_bound_;
  static type_wrapper<void> item_(...);
};

template<class T, class Base>
struct v_item : Base {
  typedef typename Base::upper_bound_ index_;
  typedef long_<index_::value + 1> upper_bound_;
  static type_wrapper<T> item_(index_);
  using Base::item_;
};

template<class T0>
struct vector1 : v_item<T0, vector0> {
};

template<class Vector, long n_>
struct v_at_impl {
  typedef long_<(Vector::lower_bound_::value + n_)> index_;
  typedef __typeof__(Vector::item_(index_())) type;
};

template<class Vector, long n_>
struct v_at : wrapped_type<typename v_at_impl<Vector, n_>::type> {
};

class C;
typedef typename v_at<vector1<C>, 0>::type result;

int main()
{
  result *p = 0;
  return p != 0;
}
