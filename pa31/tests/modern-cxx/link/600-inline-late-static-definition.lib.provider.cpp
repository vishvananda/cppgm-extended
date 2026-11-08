#include "600-inline-late-static-definition.h"
extern "C" const Point *host_address() { return &Point::origin; }
