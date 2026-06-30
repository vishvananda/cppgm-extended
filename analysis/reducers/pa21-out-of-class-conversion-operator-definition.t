template<class T>
struct sink {};

template<class Expr>
struct wrapper {
  typedef int result_type;
  operator sink<result_type>() const;
};

template<class Expr>
wrapper<Expr>::operator sink<result_type>() const {
  return sink<result_type>();
}

int consume(sink<int>);
wrapper<int> *source_wrapper();

int main() {
  return consume(*source_wrapper());
}
