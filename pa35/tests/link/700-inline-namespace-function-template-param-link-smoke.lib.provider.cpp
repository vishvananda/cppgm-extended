namespace abi_case {
template<class T> struct traits { };
template<class T> struct alloc { };

inline namespace v2 {
template<class C, class Traits, class Alloc>
struct buffer {
  int value;
};
}

template<class C, class Traits>
struct stream {
  int value;
};

template<class C, class Traits, class Alloc>
stream<C, Traits>& fetch(stream<C, Traits>& in,
                         buffer<C, Traits, Alloc>& out);

template<>
stream<char, traits<char> >&
fetch<char, traits<char>, alloc<char> >(
    stream<char, traits<char> >& in,
    buffer<char, traits<char>, alloc<char> >& out)
{
  out.value = 0;
  return in;
}
}
