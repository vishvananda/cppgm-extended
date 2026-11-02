// A memset or memcpy of a literal size, however large, whose bytes all lie in
// local arrays becomes an object clear or copy (zeroinit, copyobj): later
// passes then know what each element holds. The copy into a pointer's memory
// stays a call.
#include <cstring>

int counts(const unsigned char* text, int n)
{
	int seen[256];
	std::memset(seen, 0, sizeof seen);
	for (int i = 0; i < n; ++i)
		++seen[text[i]];
	int saved[256];
	std::memcpy(saved, seen, sizeof saved);
	int distinct = 0;
	for (int k = 0; k < 256; ++k)
		distinct += saved[k] != 0;
	return distinct;
}

void fill(int* out)
{
	int table[64];
	for (int k = 0; k < 64; ++k)
		table[k] = k * 3;
	std::memcpy(out, table, sizeof table);
}

int main()
{
	const unsigned char text[] = "abracadabra";
	int out[64];
	fill(out);
	return counts(text, 11) - 5 + out[63] - 189;
}
