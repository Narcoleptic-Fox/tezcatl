#pragma once

/* The expected verdict for every declaration is listed in
 * tests/unit/api_test.cpp. No declaration here has a trailing comment except
 * where one is the point: an ordinary trailing comment documents it. */

/// Adds two numbers.
int add(int a, int b);

int undocumented(int a);

// Subtracts b from a.
int subtract(int a, int b);

/// An attribute between the comment and the declaration.
[[nodiscard]] int checked();

/** A point in the plane. */
struct Point {
    int x; ///< Horizontal.
    int y;
    /// Length squared.
    int norm() const;
    Point() = default;
    Point(const Point&) = delete;
};

class Account {
public:
    /// Balance in cents.
    long balance() const;
    void deposit(long cents);

private:
    long cents_ = 0;
    void audit();
    struct Ledger {
        int entries;
    };
};

namespace {
int hidden();
}

static int local_helper() {
    return 1;
}

inline int twice(int v) {
    return 2 * v;
}

/// Kinds of shape.
enum class Shape { circle, square };

#define DECLARE(name) int name();
/// Made by a macro.
DECLARE(from_macro)

template <typename T> T identity(T v);

/// A distance in metres.
using Meters = double;

namespace geo {
/// Distance between two points.
double distance(const Point& a, const Point& b);
} // namespace geo

int add(int a, int b);
