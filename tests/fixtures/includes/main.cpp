#include "ring/a.h"
#include "shared/x.h"
#include "shared/y.h"
// Found through -Iinclude.
#include "lib/util.h"
#include "shared/y.h"

int main() {
    return ring_a() + common_value() + static_cast<int>(util_size());
}
