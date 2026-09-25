// Fixture for the LOC counter; its hand count is in tests/CMakeLists.txt.
#include <cstdio>

/* A block comment
   over three lines
*/
int main() {
    const char* s = "/* not a comment */"; // trailing comment

    return 0; /* opens
    and closes */
}
