template<class T, class U>
struct pair_like {};

template<class T, class U>
pair_like(T, U) -> pair_like<T, U>;
