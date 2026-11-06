// N3485 8.5.1p4,7,11: preserve the explicit prefix and inferred bound;
// expect exit 0. CXX11-007 sparse-tail neighbors.
int matrix[3][4] = {1, 2, 3, 4, 5};
int inferred[][3] = {1, 2, 3, 4};
double floating[4] = {-0.0, 1.25};
int target = 7;
int *pointers[4] = {&target};

int main()
{
    if (sizeof(inferred) != 6 * sizeof(int) || inferred[1][0] != 4 || inferred[1][2] != 0)
        return 1;
    if (matrix[0][3] != 4 || matrix[1][0] != 5 || matrix[1][1] != 0 || matrix[2][3] != 0)
        return 2;
    if (floating[1] != 1.25 || floating[2] != 0.0 || floating[3] != 0.0)
        return 3;
    if (pointers[0] != &target || pointers[1] != nullptr || pointers[3] != nullptr)
        return 4;
    return 0;
}
