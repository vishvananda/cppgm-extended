int id_calls;
int ID(int x) { ++id_calls; return x + 1; }
int G(int x) { return x + 5; }

#define EMPTY()
#define DEFER(m) m EMPTY()
#define BRIDGE() CALL
#define CALL() ID(9)
#define ID(x) x
#define F(x) 1 * x
#define G(x) 2 * x

int main() {
    return ID(DEFER(BRIDGE)()()) != 10 || id_calls != 1 || G(F)(G)(3) != 12;
}
