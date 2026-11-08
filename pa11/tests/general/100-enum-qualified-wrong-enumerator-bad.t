struct Object { enum First { A }; enum Second { B }; };
int main() { Object object; return object.First::B; }
