// Halstead fixture. Each function's counts were made by hand from its
// tokens, following the convention in docs/metrics.md: n1 distinct and N1
// total operators, n2 distinct and N2 total operands.

// n1 7 (int ( , { return + ;), N1 9; n2 3 (add a b), N2 5.
int add(int a, int b) {
    return a + b;
}

// n1 11 (bool ( const char * { return == || [ ;), N1 12;
// n2 5 (is_empty text nullptr 0 '\0'), N2 7.
bool is_empty(const char* text) {
    return text == nullptr || text[0] == '\0';
}

// The directive lines and the code #if 0 removes are not counted.
// n1 5 (int ( { return ;), N1 7; n2 3 (guarded x TWICE), N2 4.
int guarded(int x) {
#if 0
    x = x * 1000;
#endif
#define TWICE(v) ((v) * 2)
    return TWICE(x);
}

// The lambda's tokens are its own, not outer_sum's.
// n1 7 (int ( { auto = ; return), N1 10; n2 3 (outer_sum n twice), N2 5.
int outer_sum(int n) {
    // n1 7 ([ ( int { return + ;), N1 7; n2 1 (v), N2 3.
    auto twice = [](int v) { return v + v; };
    return twice(n);
}
