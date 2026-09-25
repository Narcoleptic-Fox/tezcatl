// Cyclomatic complexity fixture. The expected value of every function is
// written above it with the decision points that make it up, counted by hand
// from the definition in docs/metrics.md before Tezcatl was run on this file.

#define CHECK(x)                                                                                   \
    if (!(x))                                                                                      \
    return -1
#define IS_POSITIVE(x) ((x) > 0 ? 1 : 0)
#define DEFINE_BRANCHY(name)                                                                       \
    int name(int v) {                                                                              \
        if (v) {                                                                                   \
            return 1;                                                                              \
        }                                                                                          \
        return 0;                                                                                  \
    }

// 1: no decision points.
int straight(int a) {
    return a + 1;
}

// 3: if, else if. "else" is not a decision.
int ladder(int a) {
    if (a > 0) {
        return 1;
    } else if (a < 0) {
        return -1;
    } else {
        return 0;
    }
}

// 6: if, &&, ||, and, or.
int logic(int a, int b, int c) {
    if ((a && b) || c) {
        return 1;
    }
    return (a and b) or c;
}

// 5: for, range-for, while, do. The do-while's "while" is not a second one.
int loops(int n) {
    int total = 0;
    int values[3] = {1, 2, 3};
    for (int i = 0; i < n; ++i) {
        total += i;
    }
    for (int v : values) {
        total += v;
    }
    while (total > 100) {
        total -= 10;
    }
    do {
        --total;
    } while (total > 50);
    return total;
}

// 4: three cases. "default" is not a decision.
int choose(int k) {
    switch (k) {
    case 1:
        return 10;
    case 2:
    case 3:
        return 20;
    default:
        return 0;
    }
}

// 4: if, two catch clauses. "try" is not a decision.
int guarded(int a) {
    try {
        if (a != 0) {
            throw a;
        }
    } catch (int) {
        return 1;
    } catch (...) {
        return 2;
    }
    return 0;
}

// 3: the conditional operator and the GNU "?:".
int pick(int* p, int* q) {
    int* r = p ?: q;
    return r != nullptr ? *r : 0;
}

// 2: the if. The lambda's decisions are its own.
int outer(int a) {
    // 3: &&, ?.
    auto inner = [](int x) { return x > 0 && x < 10 ? x : 0; };
    if (a != 0) {
        return inner(a);
    }
    return 0;
}

// 1: the local class's member function is a function of its own.
int with_local(int a) {
    struct Local {
        // 2: ?.
        static int sign(int x) { return x < 0 ? -1 : 1; }
    };
    return Local::sign(a);
}

// 3: the && and || written in the macro arguments. The if and ?: inside the
// macro bodies are not written here and are not counted.
int macros(int a, int b) {
    CHECK(a > 0 && b > 0);
    return IS_POSITIVE(a || b);
}

// 2: the && written in the macro argument. The lambda in the same argument
// is a function of its own.
int macro_lambda(int a, int b) {
    // 2: ?.
    CHECK([](int x) { return x != 0 ? x : 1; }(a) && b);
    return 0;
}

// 1: its whole definition is a macro body, so none of it is written here.
DEFINE_BRANCHY(from_macro)

// 1: a preprocessor condition, code disabled by it, and an rvalue reference
// are not decisions.
int not_decisions(int a) {
#if defined(NEVER_DEFINED) && NEVER_DEFINED > 1
    if (a != 0) {
        return 0;
    }
#endif
    int&& r = a + 1;
    return r;
}

struct Flag {
    bool value;
};

// 1: "&" is not "&&".
bool operator&&(Flag a, Flag b) {
    return a.value & b.value;
}

// 1: an overloaded && is a function call and does not short-circuit.
bool combine(Flag a, Flag b) {
    return a && b;
}

// 2: the operands depend on T and an overloaded && is visible, so the
// operator is unresolved until instantiation; as written it is a &&.
template <typename T> bool dependent_and(T a, T b) {
    return a && b;
}

// 1: calls the overloaded operator by its name, which is not an operator.
template <typename T> bool call_by_name(T a, T b) {
    return operator&&(a, b);
}

// 2: a fold expression over && is one operator as written.
template <typename... T> bool all_of(T... values) {
    return (values && ...);
}

// 2: likewise a left fold over ||.
template <typename... T> bool any_of(T... values) {
    return (... || values);
}

template <typename T> struct Box {
    // 2: ?. A member function of a class template is measured as written,
    // including under clang-cl before C++20, which by default does not parse
    // template bodies until they are instantiated.
    T get_or(T fallback) const { return has ? value : fallback; }
    T value{};
    bool has = false;
};

// 3: if constexpr, ?.
template <typename T> T clamp_small(T a) {
    if constexpr (sizeof(T) > 4) {
        return a;
    } else {
        return a > 1 ? 1 : a;
    }
}

struct Holder {
    // 2: the ?: in the member initializer runs in the constructor.
    explicit Holder(int a) : value(a > 0 ? a : 0) {}
    int value;
};

// 12: eleven ifs, over the default "flagged" threshold of 10.
int eleven(int a) {
    int n = 0;
    if (a == 1) {
        ++n;
    }
    if (a == 2) {
        ++n;
    }
    if (a == 3) {
        ++n;
    }
    if (a == 4) {
        ++n;
    }
    if (a == 5) {
        ++n;
    }
    if (a == 6) {
        ++n;
    }
    if (a == 7) {
        ++n;
    }
    if (a == 8) {
        ++n;
    }
    if (a == 9) {
        ++n;
    }
    if (a == 10) {
        ++n;
    }
    if (a == 11) {
        ++n;
    }
    return n;
}
