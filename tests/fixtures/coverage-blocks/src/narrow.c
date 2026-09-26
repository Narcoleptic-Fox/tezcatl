#define IN_RANGE(v) ((v) > 0 && (v) < 10)
#include "range.h"

int narrow(int value) {
    return in_range(value);
}
