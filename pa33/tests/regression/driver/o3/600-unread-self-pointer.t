// An object that keeps a pointer to its own array (`data_ = values_`) stores
// its own address into itself. Nothing reads that pointer here, so the store
// goes, the address reaches nothing else, and the arrays live in registers:
// the function keeps no slot. (Before, the stored address counted as one that
// leaves, and every pass loaded and stored the arrays.)
struct Lanes
{
	double values_[4];
	double* data_;

	Lanes() : data_(values_) {}
	Lanes(const Lanes& other) : data_(values_)
	{
		for (int i = 0; i < 4; ++i)
			values_[i] = other.values_[i];
	}
};

double square_steps(const double* start, int steps)
{
	Lanes z;
	for (int i = 0; i < 4; ++i)
		z.values_[i] = start[i];
	for (int s = 0; s < steps; ++s)
	{
		Lanes next(z);
		for (int i = 0; i < 4; ++i)
			z.values_[i] = next.values_[i] * next.values_[i] - 1;
	}
	return z.values_[0] + z.values_[1] + z.values_[2] + z.values_[3];
}

int main()
{
	const double start[4] = { 0.5, 1, 0.25, 0 };
	const double r = square_steps(start, 3);
	return r > -4 && r < 4 ? 0 : 1;
}
