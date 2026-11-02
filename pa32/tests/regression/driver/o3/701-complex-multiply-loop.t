// The complex multiply's slow path touches no memory, so the sum's loop reads
// the vector's fields once.
struct Vec
{
	double _Complex* data;
	long size;
};

double sum_scaled(Vec* v, double _Complex w)
{
	double sum = 0;
	for (long i = 0; i < v->size; ++i)
	{
		double _Complex c = v->data[i] * w;
		sum += __real__ c;
	}
	return sum;
}
