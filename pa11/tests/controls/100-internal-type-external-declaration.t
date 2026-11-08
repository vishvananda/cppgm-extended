// expect exit 0
namespace { struct Internal { int value; }; }
int external(Internal);
int main() { return 0; }
