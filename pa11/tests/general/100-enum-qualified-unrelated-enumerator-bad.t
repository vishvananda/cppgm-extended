enum Global { Value };
struct Object {};
int main() { Object object; return object.Global::Value; }
