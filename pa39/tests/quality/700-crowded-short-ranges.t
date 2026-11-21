// A product of 28-column matrices. The vectorized inner loop runs only when
// the result row overlaps none of the 28 rows of m2 and the row of m1: the
// 29 overlap tests are a chain of short values, each defined and read again
// within a few instructions, while the row pointers that the loop reads stay
// live across it and fill the general registers. Each short value keeps a
// register, which one register holds in turn, instead of a store and a reload
// on the chain: the row pointers it displaces are reloaded where they are read.
#define SIZE 28
extern "C" __attribute__((noinline)) int** mmult(int** m1, int** m2, int** m3) {
    int i, j, k, val;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            val = 0;
            for (k = 0; k < SIZE; k++)
                val += m1[i][k] * m2[k][j];
            m3[i][j] = val;
        }
    }
    return m3;
}
