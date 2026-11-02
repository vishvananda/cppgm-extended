// A virtual call tests for the function it names and calls it directly,
// which inlines it; a function with an overrider keeps its plain call.
struct Lone
{
	int count;
	virtual int next() { return ++count; }
};

struct Base
{
	virtual int value() { return 1; }
};

struct Derived : Base
{
	int value() { return 2; }
};

int lone(Lone* p)
{
	return p->next() + 1;
}

int base(Base* p)
{
	return p->value() + 1;
}

int main()
{
	Lone l;
	l.count = 2;
	Derived d;
	return lone(&l) + base(&d);
}
