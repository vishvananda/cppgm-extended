// expect reject
struct View { template<class T> explicit operator T*() const; };
bool equal(View value, const unsigned char* pointer) { return value == pointer; }
