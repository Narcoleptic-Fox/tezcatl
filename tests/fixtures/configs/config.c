/* Compiled twice: first with FIRST and HEAVY, then plainly. HEAVY includes a
   generated header of 20,000 functions so the first unit parses slowest. */
#ifdef HEAVY
#include "heavy.h"
#endif

int decide(int x) {
#ifdef FIRST
    if (x > 0) {
        return 1;
    }
    if (x < 0) {
        return -1;
    }
#endif
    return 0;
}
