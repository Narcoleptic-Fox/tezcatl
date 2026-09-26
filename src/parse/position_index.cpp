#include "parse/position_index.hpp"

#include <algorithm>

namespace tezcatl::parse {

PositionIndex::PositionIndex(std::vector<Position> declarations, std::vector<Extent> typedefs)
    : declarations_(std::move(declarations)), typedefs_(std::move(typedefs)) {
    // Both searches below require sorted input.
    std::ranges::sort(declarations_);
    std::ranges::sort(typedefs_);
}

bool PositionIndex::declaration_between(const FileKey& file, Gap gap) const {
    // The first declaration after `gap.after` in this file, if any; a later
    // file's declarations sort after all of this file's, so the file must be
    // checked too.
    const auto first =
        std::ranges::upper_bound(declarations_, Position{.file = file, .offset = gap.after});
    return first != declarations_.end() && first->file == file && first->offset < gap.before;
}

bool PositionIndex::inside_typedef(const FileKey& file, unsigned offset) const {
    return std::ranges::any_of(
        std::ranges::equal_range(typedefs_, file, {}, &Extent::file),
        [offset](const Extent& extent) { return offset > extent.begin && offset < extent.end; });
}

} // namespace tezcatl::parse
