#include "api.hpp"
#include "elsewhere.hpp"

// A comment on the definition documents nothing in the header, which is
// where a reader of the API looks.
int undocumented(int a) {
    return a;
}

// libclang attaches this comment to the header's declaration too, as a
// comment of a redeclaration. It is in another file, so it does not count.
int defined_elsewhere(int a) {
    return a;
}

// Declared nowhere else: not in a header, so not part of the API.
int only_in_source(int a) {
    return a;
}
