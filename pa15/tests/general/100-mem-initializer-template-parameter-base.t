// 14.6.2p3: a dependent base is not searched by unqualified lookup, so the
// mem-initializer-id still names the template parameter in an instance
// whose base inherits the injected-class-name of a template of that name.
template<int N, class T> struct Domain
{
	int value;
	Domain(int v) : value(v + N) {}
};

struct Interval : Domain<1, int>
{
	Interval(int x) : Domain<1, int>(x) {}
};

template<class Domain> struct Wrap : public Domain
{
	Wrap() : Domain(1) {}
};

template<class Domain> struct Own
{
	int Domain_;
	struct Inner
	{
	};
	Own() : Domain_(3) {}
};

int main()
{
	Wrap<Interval> w;
	Own<int> o;
	return w.value == 2 && o.Domain_ == 3 ? 0 : 1;
}
