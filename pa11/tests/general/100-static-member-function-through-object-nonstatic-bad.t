// 5.2.5p4: a non-static member function named through an object is only
// the callee of a call.
struct S
{
	void n() {}
};

void (*r)();

int main()
{
	S s;
	r = s.n;
}
