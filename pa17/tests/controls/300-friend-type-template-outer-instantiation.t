// N3485 14p1,14.5.4: distinguish a friend template from an ordinary friend type in a class template.
// expect reject
struct Owner { typedef int Type; }; template<class U> struct Grant { template<class T> friend typename U::Type; }; Grant<Owner> value;
