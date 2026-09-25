#include "calc.hpp"

int classify(int value) {
    if (value < 0) {
        return -1;
    }
    if (value == 0) {
        return 0;
    }
    return 1;
}

int never_called(int value) {
    return value * 2;
}
