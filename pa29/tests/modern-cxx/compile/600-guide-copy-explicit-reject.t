template<class T> struct Box { Box(T); };
template<class T> explicit Box(T) -> Box<T>;
Box value = {3};
