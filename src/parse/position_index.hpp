#pragma once

#include <array>
#include <compare>
#include <utility>
#include <vector>

namespace tezcatl::parse {

/// A file's identity as clang_File_isEqual compares it: its unique ID.
using FileKey = std::array<unsigned long long, 3>;

/// Where one scope's declarations and typedefs are written, sorted so that
/// the questions the documentation check asks are binary searches. Asking
/// every sibling of every declaration was quadratic in the size of a scope,
/// and a C unit's top level holds every declaration of every header it
/// includes. Plain data: built from libclang cursors in api.cpp, and tested
/// on its own.
class PositionIndex {
public:
    /// Where a declaration's name is written.
    struct Position {
        FileKey file{};
        unsigned offset = 0;
        friend auto operator<=>(const Position&, const Position&) = default;
    };
    /// Where a typedef is written, from its first to its last byte.
    struct Extent {
        FileKey file{};
        unsigned begin = 0;
        unsigned end = 0;
        friend auto operator<=>(const Extent&, const Extent&) = default;
    };
    /// Offsets strictly between which a declaration is looked for: named, so
    /// the two cannot be passed the wrong way round.
    struct Gap {
        unsigned after = 0;
        unsigned before = 0;
    };

    /// In any order: the index sorts them.
    PositionIndex(std::vector<Position> declarations, std::vector<Extent> typedefs);

    /// Whether a declaration is written in `file` strictly inside `gap`.
    [[nodiscard]] bool declaration_between(const FileKey& file, Gap gap) const;

    /// Whether `offset` in `file` lies strictly inside a typedef.
    [[nodiscard]] bool inside_typedef(const FileKey& file, unsigned offset) const;

private:
    std::vector<Position> declarations_;
    std::vector<Extent> typedefs_;
};

} // namespace tezcatl::parse
