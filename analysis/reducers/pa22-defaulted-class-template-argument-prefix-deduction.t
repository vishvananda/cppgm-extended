template<class T>
struct less
{
};

template<class T>
struct vec
{
  typedef T value_type;
};

template<class T, class Container = vec<T>, class Compare = less<typename Container::value_type> >
struct queue
{
};

template<class T, class Container>
int probe(queue<T, Container> &)
{
  return 7;
}

int main()
{
  queue<int> q;
  return probe(q) == 7 ? 0 : 1;
}
