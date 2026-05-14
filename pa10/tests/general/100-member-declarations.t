struct C { C(); ~C(); operator int() const; int operator[](int i) const; C& operator=(const C&) = default; };
