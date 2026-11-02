// An encoder's bit loop reads tables at a byte of the input: those loads leave
// the loop. A byte index stays inside each 256-entry local table, the input
// byte was read before the loop with no call between, and the loop's stores go
// to storage malloc returned, which no pointer from before the call reaches,
// so they cannot change the byte. The loop may leave through the overflow
// call, but every load it moves may be read on entry.
#include <cstdio>
#include <cstdlib>
#include <cstring>

unsigned long encode(const unsigned char* data, unsigned long n)
{
	unsigned char* out = static_cast<unsigned char*>(std::malloc(n));
	unsigned long code[256];
	unsigned char length[256];
	for (int k = 0; k < 256; ++k)
	{
		code[k] = static_cast<unsigned long>(k) * 5;
		length[k] = static_cast<unsigned char>(k % 7 + 1);
	}
	unsigned long used = 0;
	unsigned char acc = 0;
	int bit = -1;
	const unsigned char* dptr = data;
	for (unsigned long j = 0; j < n; ++j)
	{
		unsigned long mask = 1ul << (length[*dptr] - 1);
		for (int i = 0; i < length[*dptr]; ++i)
		{
			if (bit == 7)
			{
				out[used] = acc;
				++used;
				if (used == n)
				{
					std::fputs("no compression\n", stderr);
					std::exit(1);
				}
				bit = 0;
				acc = 0;
			}
			else
			{
				++bit;
				acc = static_cast<unsigned char>(acc << 1);
			}
			if (code[*dptr] & mask)
				acc |= 1;
			mask >>= 1;
		}
		++dptr;
	}
	unsigned long sum = 0;
	for (unsigned long k = 0; k < used; ++k)
		sum += out[k];
	std::free(out);
	return used * 1000 + sum % 1000;
}

int main()
{
	const unsigned char text[] = "abracadabra";
	std::printf("%lu\n", encode(text, 11));
	return 0;
}
