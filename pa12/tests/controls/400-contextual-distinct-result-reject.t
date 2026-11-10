struct Count { operator unsigned() const; operator int() const; };
void f(Count n) { int *p = new int[n]; delete [] p; }
