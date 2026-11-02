// A small class whose members leave a gap of padding moves as one object
// copy (one wide load and store in the back end); a class whose members
// tile it still moves member by member.
struct Operand
{
	unsigned char kind, sub;
	unsigned index;
};

struct Pair
{
	unsigned first, second;
};

void move(Operand* to, const Operand* from)
{
	*to = *from;
}

void move(Pair* to, const Pair* from)
{
	*to = *from;
}

int main()
{
	Operand a = { 1, 2, 3 };
	Operand b;
	move(&b, &a);
	Pair c = { 4, 5 };
	Pair d;
	move(&d, &c);
	return b.kind + b.sub + static_cast<int>(b.index + d.first + d.second) - 15;
}
