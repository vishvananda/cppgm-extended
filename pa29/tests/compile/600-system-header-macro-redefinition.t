#define REPLACED 1
#define FUNCTION(x) ((x) + 1)
#define SHAPE 1
#define INDIRECT 1
static_assert(REPLACED == 1, "original definition");
#include <redefine.h>
static_assert(REPLACED == 7, "system header definition");
static_assert(FUNCTION(3, 4) == 7, "new function signature");
static_assert(SHAPE(5) == 5, "object macro becomes function macro");
static_assert(INDIRECT == 99, "nested quoted include inherits system status");
#define REPLACED 7
int main() { return 0; }
