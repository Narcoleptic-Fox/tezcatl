#define IN_RANGE(v) ((v) > -100 && (v) < 100 && (v) != 50)
#include "range.h"

int wide(int value) {
    return in_range(value);
}
