#include "shared.hpp"
#include "util/detail.hpp"

namespace {
int helper(int v) { return v * 2; }
} // namespace

namespace geo {
int declared_only(int v) { return helper(v) + util::one(); }
} // namespace geo

class Shape {
public:
    Shape() = default;
    explicit Shape(int sides) : sides_(sides) {}
    ~Shape() {}
    Shape(const Shape&) = delete;
    operator int() const { return sides_; }
    template <typename T> T scaled(T factor) const { return static_cast<T>(sides_) * factor; }

private:
    int sides_ = 0;
};

int use_lambda() {
    auto twice = [](int v) { return v * 2; };
    return twice(3);
}

#define DEFINE_GETTER(name, value) int name() { return value; }
DEFINE_GETTER(forty_two, 42)

// "= default" is not a body written in the project, even where the compiler
// writes one: out of line, or when the function is used (copy_of copies).
struct Counted {
    Counted();
    ~Counted();
    Counted(const Counted&) = default;
};
Counted::Counted() = default;
Counted::~Counted() = default;
Counted copy_of(const Counted& original) { return original; }
