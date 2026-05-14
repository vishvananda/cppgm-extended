template<class T>
struct wrap {};

template<class T>
explicit wrap(T) -> wrap<T>;
