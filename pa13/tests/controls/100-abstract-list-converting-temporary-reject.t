// N3485 10.4p2, 8.5.4p3: even a usable converting constructor cannot create
// an object of abstract type for a reference default argument. expect reject
struct Abstract { Abstract(int) {} virtual int value() const = 0; };
void fail(const Abstract& = { 7 });
