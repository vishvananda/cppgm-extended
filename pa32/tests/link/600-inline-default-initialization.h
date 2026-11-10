extern int constructed;
int observe(int);
struct Cell {
    int value;
    Cell() : value(observe(41)) {}
    explicit Cell(int n) : value(observe(n)) {}
};
struct InlineCells {
    static inline Cell default_cell;
    static inline Cell explicit_cell = Cell(43);
};
