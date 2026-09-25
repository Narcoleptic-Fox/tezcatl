#include "shared.hpp"
#include <cpuid.h> // only in clang's resource directory, never the OS's
int from_b() { return static_cast<int>(geo::area(2, 3)); }
