enum class Byte:unsigned char {};
int main() { struct Source {operator unsigned char()const{return 7;}};Byte value{Source{}}; }
