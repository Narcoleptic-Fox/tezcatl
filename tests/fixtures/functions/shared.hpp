#pragma once
#include <stddef.h>

namespace geo {

struct Point {
    int x = 0;
    int y = 0;
    int sum() const { return x + y; }
};

inline size_t area(size_t w, size_t h) {
    return w * h;
}

int declared_only(int);

} // namespace geo
