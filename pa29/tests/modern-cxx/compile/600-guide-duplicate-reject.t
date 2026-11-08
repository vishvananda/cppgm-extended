template<class T> struct Box {};
template<class T> Box(T) -> Box<T>;
template<class U> Box(U) -> Box<U>;
