// Narrowing a word load to its second byte must retain the full address.
__attribute__((noinline)) unsigned byte_field(const unsigned* word)
{
    return (*word >> 8) & 255;
}

__attribute__((noinline)) unsigned half_field(const unsigned long long* word)
{
    return (*word >> 32) & 65535;
}

int main(int argc, char**)
{
    unsigned word = 0x12345678u + argc;
    unsigned long long wide = 0x123456789abcdef0ull + argc;
    return byte_field(&word) != 0x56 || half_field(&wide) != 0x5678;
}
