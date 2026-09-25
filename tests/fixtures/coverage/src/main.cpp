#include "calc.hpp"

// classify(0) is never called, so one branch of classify is never taken;
// never_called is never called; clamp is instantiated for int and double.
int main() {
    int total = classify(5) + classify(-3);
    total += clamp(7, 0, 5);
    total += static_cast<int>(clamp(0.5, 1.0, 2.0));
    return total == 6 ? 0 : 1;
}
