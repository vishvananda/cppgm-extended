// A small class moves by typed accesses of its members, so the store does
// not hide a table the loop reads; a union keeps its object copy.
struct Vec
{
	double x, y;
};

struct Table
{
	Vec* cells;
	long size;
};

union Either
{
	long whole;
	double real;
};

struct Boxed
{
	Either value;
};

void fill(Table* t, const Vec* from)
{
	for (long i = 0; i < t->size; ++i)
		t->cells[i] = from[i];
}

void boxed(Boxed* to, const Boxed* from)
{
	*to = *from;
}

int main()
{
	Vec a[2] = { { 1, 2 }, { 3, 4 } };
	Vec b[2];
	Table t = { b, 2 };
	fill(&t, a);
	Boxed c, d;
	d.value.whole = 3;
	boxed(&c, &d);
	return static_cast<int>(b[1].y) + static_cast<int>(c.value.whole);
}
