#include "metrics/loc.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>

namespace tezcatl::metrics {

LocCounts& LocCounts::operator+=(const LocCounts& other) noexcept {
    physical += other.physical;
    blank += other.blank;
    comment += other.comment;
    code += other.code;
    return *this;
}

namespace {

constexpr std::string_view utf8_bom = "\xEF\xBB\xBF";

// The longest raw string delimiter the standard allows.
constexpr std::size_t max_raw_delimiter = 16;

constexpr bool is_space(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

constexpr bool is_digit(char c) noexcept {
    return c >= '0' && c <= '9';
}

constexpr bool is_identifier_char(char c) noexcept {
    return is_digit(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

constexpr bool is_exponent_marker(char c) noexcept {
    return c == 'e' || c == 'E' || c == 'p' || c == 'P';
}

bool is_raw_string_prefix(std::string_view token) noexcept {
    constexpr std::array<std::string_view, 5> prefixes{"R", "LR", "uR", "UR", "u8R"};
    return std::ranges::find(prefixes, token) != prefixes.end();
}

// A single forward pass over the source. The position never rests on the
// start of a line splice outside a raw string: step() consumes splices as it
// moves, finishing the physical line each one ends.
class LineClassifier {
public:
    explicit LineClassifier(std::string_view source) : source_(source) {}

    std::vector<LineKind> run() {
        if (source_.starts_with(utf8_bom)) {
            pos_ = utf8_bom.size();
        }
        consume_splices();
        while (pos_ < source_.size()) {
            const char c = at(pos_);
            if (c == '\n') {
                end_line();
                continue;
            }
            line_started_ = true;
            switch (state_) {
            case State::code:
                scan_code(c);
                break;
            case State::line_comment:
                if (!is_space(c)) {
                    has_comment_ = true;
                }
                step();
                break;
            case State::block_comment:
                scan_block_comment(c);
                break;
            case State::string_literal:
                scan_quoted(c, '"');
                break;
            case State::char_literal:
                scan_quoted(c, '\'');
                break;
            case State::raw_string:
                scan_raw_string(c);
                break;
            }
        }
        if (line_started_) {
            finish_line();
        }
        return std::move(lines_);
    }

private:
    enum class State : std::uint8_t {
        code,
        line_comment,
        block_comment,
        string_literal,
        char_literal,
        raw_string
    };

    // Every read of the source goes through here, so no read is unchecked.
    [[nodiscard]] char at(std::size_t pos) const {
        return pos < source_.size() ? source_.at(pos) : '\0';
    }

    // Length of the backslash-newline sequence starting at pos, or 0.
    [[nodiscard]] std::size_t splice_length(std::size_t pos) const {
        if (at(pos) != '\\') {
            return 0;
        }
        if (at(pos + 1) == '\n') {
            return 2;
        }
        if (at(pos + 1) == '\r' && at(pos + 2) == '\n') {
            return 3;
        }
        return 0;
    }

    // The next logical character, looking through any splices.
    [[nodiscard]] char peek() const {
        std::size_t next = pos_ + 1;
        if (state_ != State::raw_string) {
            while (const std::size_t length = splice_length(next)) {
                next += length;
            }
        }
        return at(next);
    }

    void consume_splices() {
        if (state_ == State::raw_string) {
            return;
        }
        while (const std::size_t length = splice_length(pos_)) {
            finish_line();
            pos_ += length;
        }
    }

    void step() {
        ++pos_;
        consume_splices();
    }

    void finish_line() {
        if (has_code_) {
            lines_.push_back(LineKind::code);
        } else if (has_comment_) {
            lines_.push_back(LineKind::comment);
        } else {
            lines_.push_back(LineKind::blank);
        }
        has_code_ = false;
        has_comment_ = false;
        line_started_ = false;
    }

    void end_line() {
        finish_line();
        // A newline ends a line comment. Ordinary literals cannot span lines,
        // so an unterminated one is recovered from here rather than allowed
        // to swallow the rest of the file.
        if (state_ == State::line_comment || state_ == State::string_literal ||
            state_ == State::char_literal) {
            state_ = State::code;
        }
        end_token();
        step();
    }

    void end_token() {
        token_.clear();
        in_number_ = false;
    }

    // Enters a comment on the two-character opener at pos_. Both characters
    // are marked because a splice may put them on different physical lines.
    void open_comment(State comment) {
        end_token();
        state_ = comment;
        has_comment_ = true;
        step();
        has_comment_ = true;
        step();
    }

    void scan_code(char c) {
        if (is_space(c)) {
            end_token();
            step();
            return;
        }
        if (c == '/' && peek() == '/') {
            open_comment(State::line_comment);
            return;
        }
        if (c == '/' && peek() == '*') {
            open_comment(State::block_comment);
            return;
        }
        has_code_ = true;
        if (c == '"') {
            open_string();
            return;
        }
        if (c == '\'') {
            if (in_number_ && is_identifier_char(peek())) {
                step(); // digit separator, as in 1'000
                return;
            }
            end_token();
            state_ = State::char_literal;
            step();
            return;
        }
        track_token(c);
        step();
    }

    // Tracks the identifier or preprocessing number being scanned, which is
    // all that is needed to tell a raw string prefix and a digit separator.
    void track_token(char c) {
        const bool continues_number =
            in_number_ && (is_identifier_char(c) || c == '.' ||
                           ((c == '+' || c == '-') && is_exponent_marker(token_.back())));
        if (continues_number) {
            token_ += c;
        } else if (is_identifier_char(c)) {
            if (token_.empty()) {
                in_number_ = is_digit(c);
            }
            token_ += c;
        } else if (c == '.' && is_digit(peek())) {
            end_token();
            in_number_ = true;
            token_ += c;
        } else {
            end_token();
        }
    }

    void open_string() {
        if (is_raw_string_prefix(token_) && open_raw_string()) {
            return;
        }
        end_token();
        state_ = State::string_literal;
        step();
    }

    // At the opening quote of a raw string. Splices are reverted inside raw
    // strings, so the delimiter is read from the raw characters.
    bool open_raw_string() {
        std::size_t pos = pos_ + 1;
        std::string delimiter;
        while (pos < source_.size() && at(pos) != '(') {
            const char c = at(pos);
            if (delimiter.size() == max_raw_delimiter || is_space(c) || c == '\n' || c == ')' ||
                c == '\\' || c == '"') {
                return false; // ill-formed; scan it as an ordinary string
            }
            delimiter += c;
            ++pos;
        }
        if (pos == source_.size()) {
            return false;
        }
        end_token();
        terminator_ = ")" + delimiter + "\"";
        state_ = State::raw_string;
        pos_ = pos + 1;
        return true;
    }

    void scan_raw_string(char c) {
        if (source_.substr(pos_).starts_with(terminator_)) {
            has_code_ = true;
            state_ = State::code;
            pos_ += terminator_.size() - 1;
            step();
            return;
        }
        if (!is_space(c)) {
            has_code_ = true;
        }
        ++pos_;
    }

    void scan_quoted(char c, char quote) {
        if (!is_space(c)) {
            has_code_ = true;
        }
        if (c == '\\') {
            step();
            if (pos_ < source_.size() && at(pos_) != '\n') {
                step(); // the escaped character, whatever it is
            }
            return;
        }
        if (c == quote) {
            state_ = State::code;
        }
        step();
    }

    void scan_block_comment(char c) {
        if (c == '*' && peek() == '/') {
            has_comment_ = true;
            step();
            has_comment_ = true;
            state_ = State::code;
            step();
            return;
        }
        if (!is_space(c)) {
            has_comment_ = true;
        }
        step();
    }

    std::string_view source_;
    std::size_t pos_ = 0;
    State state_ = State::code;
    std::vector<LineKind> lines_;
    bool has_code_ = false;
    bool has_comment_ = false;
    bool line_started_ = false;
    std::string token_;
    bool in_number_ = false;
    std::string terminator_;
};

} // namespace

std::vector<LineKind> classify_lines(std::string_view source) {
    return LineClassifier{source}.run();
}

LocCounts count_lines(std::string_view source) {
    LocCounts counts;
    for (const LineKind kind : classify_lines(source)) {
        ++counts.physical;
        switch (kind) {
        case LineKind::blank:
            ++counts.blank;
            break;
        case LineKind::comment:
            ++counts.comment;
            break;
        case LineKind::code:
            ++counts.code;
            break;
        }
    }
    return counts;
}

} // namespace tezcatl::metrics
