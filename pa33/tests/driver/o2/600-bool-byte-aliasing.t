// A bool object's byte is read and written through bool glvalues alone:
// its store changes no int, and an int's store changes no bool, so the
// second load of each folds. A character glvalue accesses any object's
// bytes: a load after its store stays.
__attribute__((noinline)) int across_bool(int* n, bool* flag)
{
	int before = *n;
	*flag = true;
	return before + *n;
}

__attribute__((noinline)) int bool_across_int(bool* flag, int* n)
{
	bool before = *flag;
	*n = 0x01010101;
	return before + *flag;
}

__attribute__((noinline)) int across_char(int* n, unsigned char* c)
{
	int before = *n;
	*c = 2;
	return before + *n;
}

__attribute__((noinline)) int bool_across_char(bool* flag, char* c)
{
	bool before = *flag;
	*c = 1;
	return before + *flag;
}

int main()
{
	volatile int seed = 5;
	int n = seed;
	bool flag = false;
	if (across_bool(&n, &flag) != 10 || !flag)
		return 1;
	if (bool_across_int(&flag, &n) != 2 || n != 0x01010101)
		return 2;
	n = 0x100;
	if (across_char(&n, reinterpret_cast<unsigned char*>(&n)) != 0x100 + 0x102)
		return 3;
	flag = false;
	if (bool_across_char(&flag, reinterpret_cast<char*>(&flag)) != 1)
		return 4;
	return 0;
}
