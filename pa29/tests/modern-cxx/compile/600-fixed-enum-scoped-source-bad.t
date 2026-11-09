enum class Byte:unsigned char {};
int main() { enum class Source:unsigned char{Seven=7};Byte value{Source::Seven}; }
