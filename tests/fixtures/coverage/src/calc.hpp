#pragma once

int classify(int value);
int never_called(int value);

template <typename T> T clamp(T value, T low, T high) {
    if (value < low) {
        return low;
    }
    return value > high ? high : value;
}
