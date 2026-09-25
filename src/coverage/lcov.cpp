#include "coverage/readers.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tezcatl::coverage {

namespace fs = std::filesystem;

namespace {

class LcovReader {
public:
    LcovReader(fs::path base, fs::path source, CoverageData& data)
        : base_(std::move(base)), source_(std::move(source)), data_(&data) {}

    void read_line(std::string_view line) {
        ++line_number_;
        if (line.ends_with('\r')) {
            line.remove_suffix(1);
        }
        const std::size_t colon = line.find(':');
        const std::string_view tag = line.substr(0, colon);
        const std::string_view value =
            colon == std::string_view::npos ? std::string_view{} : line.substr(colon + 1);
        if (tag == "SF") {
            const fs::path file{value};
            const fs::path path = resolve_recorded_path(file, base_);
            current_ = &(*data_)[path];
            if (current_->has_totals()) {
                fail("line data for " + path.string() + ", which also has llvm-cov totals");
            }
        } else if (tag == "end_of_record") {
            current_ = nullptr;
        } else if (tag == "DA") {
            line_record(value);
        } else if (tag == "BRDA") {
            branch_record(value);
        } else if (tag == "FN") {
            function_record(value);
        } else if (tag == "FNDA") {
            function_hits(value);
        } else if (tag == "FNA") {
            // lcov 2.2: FNA:<index>,<hits>,<name>
            function_hits(after_field(value, 1));
        }
        // TN, VER, FNL and the totals records carry nothing that is not
        // recomputed from the records above.
    }

private:
    [[noreturn]] void fail(std::string_view problem) const {
        throw std::runtime_error(source_.generic_string() + ':' + std::to_string(line_number_) +
                                 ": " + std::string{problem});
    }

    [[nodiscard]] FileRecord& current() const {
        if (current_ == nullptr) {
            fail("coverage record outside an SF ... end_of_record block");
        }
        return *current_;
    }

    [[nodiscard]] std::uint64_t number(std::string_view text) const {
        std::uint64_t value = 0;
        const char* const first = std::to_address(text.begin());
        const char* const last = std::to_address(text.end());
        const auto [end, error] = std::from_chars(first, last, value);
        if (error != std::errc{} || end != last) {
            fail("expected a number, found '" + std::string{text} + "'");
        }
        return value;
    }

    [[nodiscard]] unsigned line_of(std::string_view text) const {
        const std::uint64_t value = number(text);
        if (value == 0 || value > std::numeric_limits<unsigned>::max()) {
            fail("line number out of range: " + std::string{text});
        }
        return static_cast<unsigned>(value);
    }

    // The fields of a comma-separated record; the last one may contain
    // commas itself (a demangled function name), so at most `count` fields
    // are split off and the rest is returned as the last.
    [[nodiscard]] std::vector<std::string_view> fields(std::string_view value,
                                                       std::size_t count) const {
        std::vector<std::string_view> result;
        while (result.size() + 1 < count) {
            const std::size_t comma = value.find(',');
            if (comma == std::string_view::npos) {
                fail("too few fields");
            }
            result.push_back(value.substr(0, comma));
            value.remove_prefix(comma + 1);
        }
        result.push_back(value);
        return result;
    }

    [[nodiscard]] std::string_view after_field(std::string_view value, std::size_t skip) const {
        return fields(value, skip + 1).back();
    }

    // DA:<line>,<hits>[,<checksum>]
    void line_record(std::string_view value) {
        std::vector<std::string_view> parts = fields(value, 2);
        const std::size_t checksum = parts.back().find(',');
        if (checksum != std::string_view::npos) {
            parts.back() = parts.back().substr(0, checksum);
        }
        current().add_line(line_of(parts.at(0)), number(parts.at(1)));
    }

    // BRDA:<line>,[e]<block>,<branch>,<taken>, where taken is "-" if the
    // branch's condition was never evaluated.
    void branch_record(std::string_view value) {
        const std::vector<std::string_view> parts = fields(value, 4);
        const std::string_view taken = parts.at(3);
        const std::string id = std::string{parts.at(1)} + ',' + std::string{parts.at(2)};
        current().add_branch(line_of(parts.at(0)), id, taken == "-" ? 0 : number(taken));
    }

    // FN:<line>,<name> or, in lcov 2, FN:<line>,<end line>,<name>.
    void function_record(std::string_view value) {
        std::vector<std::string_view> parts = fields(value, 2);
        (void)line_of(parts.at(0));
        std::string_view name = parts.at(1);
        const std::size_t comma = name.find(',');
        if (comma != std::string_view::npos &&
            name.substr(0, comma).find_first_not_of("0123456789") == std::string_view::npos &&
            comma > 0) {
            name.remove_prefix(comma + 1);
        }
        current().add_function(std::string{name}, 0);
    }

    // FNDA:<hits>,<name>
    void function_hits(std::string_view value) {
        const std::vector<std::string_view> parts = fields(value, 2);
        current().add_function(std::string{parts.at(1)}, number(parts.at(0)));
    }

    fs::path base_;
    fs::path source_;
    CoverageData* data_;
    FileRecord* current_ = nullptr;
    std::size_t line_number_ = 0;
};

} // namespace

void read_lcov(std::istream& in, const fs::path& base, const fs::path& source, CoverageData& data) {
    LcovReader reader{base, source, data};
    std::string line;
    while (std::getline(in, line)) {
        reader.read_line(line);
    }
}

} // namespace tezcatl::coverage
