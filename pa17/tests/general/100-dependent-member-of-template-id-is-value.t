// 14.6p2: a member of a template-id depending on a template parameter is a
// value without `typename`, though the primary template is known and the
// parser's lexical fallback would call a name holding C, Y, E or T a type.
template<typename T> struct traits;
template<typename T> struct traits<const T> : traits<T>
{
};
template<int R, int C> struct size
{
	enum
	{
		value = R * C
	};
};

template<typename X> struct shape
{
	enum
	{
		area = size<traits<X>::Rows, traits<X>::Cols>::value,
		elements = size<traits<X>::Elements, traits<X>::T>::value,
		other = size<traits<X>::Y, traits<const X>::Cols>::value
	};
};

template<> struct traits<int>
{
	enum
	{
		Rows = 2,
		Cols = 3,
		Elements = 6,
		T = 7,
		Y = 8
	};
};

int main()
{
	return shape<int>::area == 6 && shape<int>::elements == 42 && shape<int>::other == 24 ? 0 : 1;
}
