// 14.6.2.3: an enumerator whose value depends on a template parameter has
// none in the template's definition, where only its position is known, and
// one following it depends on it. Its value is converted to the parameter
// of a template-id only in an instantiation: 14.3.2p5 narrows no bool here
// although the third enumerator is not the first two.
template<bool B> struct flag
{
	enum
	{
		value = B
	};
	typedef int type;
};

struct X
{
	enum
	{
		F = 512,
		Cost = 5
	};
};

template<class T> struct S
{
	enum
	{
		Cost = T::Cost,
		Added = 3,
		Read = (int(Cost) + 1) * Cost,
		Stored = Added * Cost,
		Packet = (int(T::F) & 512) || (int(Read) < int(Stored))
	};
	typedef flag<Packet> t;
	typedef typename t::type u;
	char a[Packet];
	int bits : Packet;
};

template<class T> struct Next
{
	enum
	{
		Zero = int(sizeof(T)) - 6,
		One,
		Two
	};
	typedef flag<Two> t;
	int f(int x)
	{
		switch (x)
		{
		case Two:
			return 1;
		case One:
			return 2;
		}
		return 0;
	}
};

template<class T> int local()
{
	enum
	{
		A = sizeof(T),
		B,
		C
	};
	char buffer[C];
	static_assert(C == sizeof(T) + 2, "");
	return sizeof(buffer);
}

template<class D> struct Dimension
{
	enum
	{
		Size = (D::value * D::value - D::value) / 2 + 1 / D::value
	};
};
struct Two
{
	enum
	{
		value = 2
	};
};

int main()
{
	return S<X>::t::value == 1 && Next<int>::t::value == 0 && Next<int>().f(0) == 1 && local<int>() == 6 &&
			Dimension<Two>::Size == 1
		? 0
		: 1;
}
