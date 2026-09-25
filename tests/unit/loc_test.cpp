#include "metrics/loc.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <vector>

// Every expected value here is counted by hand from the snippet, never taken
// from Tezcatl's output. Most cases are built so that one specific mistake
// flips one specific line; the comment on each says which.

using tezcatl::metrics::classify_lines;
using tezcatl::metrics::count_lines;
using tezcatl::metrics::LineKind;
using tezcatl::metrics::LocCounts;

namespace {

constexpr LineKind blank = LineKind::blank;
constexpr LineKind comment = LineKind::comment;
constexpr LineKind code = LineKind::code;

std::vector<LineKind> kinds(std::string_view source) {
    return classify_lines(source);
}

} // namespace

TEST_CASE("line counting basics", "[loc]") {
    CHECK(kinds("").empty());
    CHECK(kinds("int x;\n") == std::vector{code});
    CHECK(kinds("int x;") == std::vector{code}); // no final newline still counts
    CHECK(kinds("\n\n") == std::vector{blank, blank});
    CHECK(kinds("  \t \n") == std::vector{blank});
    CHECK(kinds("int x;\r\n\r\n// c\r\n") == std::vector{code, blank, comment});
}

TEST_CASE("comments and code on the same line", "[loc]") {
    CHECK(kinds("// c\nint x; // t\n") == std::vector{comment, code});
    CHECK(kinds("/* a */ int b;\n") == std::vector{code});
    CHECK(kinds("int b; /* start\n end */\n") == std::vector{code, comment});
}

TEST_CASE("block comments span lines and whitespace inside them is blank", "[loc]") {
    CHECK(kinds("/* a\n\n b\n */\nint y; /* c */\n") ==
          std::vector{comment, blank, comment, comment, code});
    // "/*/" does not close the comment it opens.
    CHECK(kinds("/*/ still\n comment */\nint c;\n") == std::vector{comment, comment, code});
    // Unterminated at end of file: everything after the opener is comment.
    CHECK(kinds("/* open\nint x;\n") == std::vector{comment, comment});
}

TEST_CASE("comment markers inside literals are not comments", "[loc]") {
    CHECK(kinds("s = \"/*\";\nint z;\n") == std::vector{code, code});
    CHECK(kinds("s = \"// no\";\n") == std::vector{code});
    // An escaped quote does not end the string, so "/*" stays inside it.
    CHECK(kinds("s = \"\\\"/*\";\nint w;\n") == std::vector{code, code});
    // An escaped quote in a character literal does not end it either.
    CHECK(kinds("c = '\\''; /* x\n*/\n") == std::vector{code, comment});
    CHECK(kinds("q = '\"'; /* x\n*/\n") == std::vector{code, comment});
}

TEST_CASE("digit separators are not character literals", "[loc]") {
    // Read as a character literal, the ' would hide the "/*" on line 1 and
    // line 2 would become code.
    CHECK(kinds("n = 1'0; /* c\n*/\n") == std::vector{code, comment});
    CHECK(kinds("n = 0x1'F'F; /* c\n*/\n") == std::vector{code, comment});
}

TEST_CASE("raw string literals", "[loc]") {
    // Comment markers and blank lines inside are string content, not comments.
    CHECK(kinds("auto r = R\"x(\n// in\n\n/* in\n)x\";\nint v;\n") ==
          std::vector{code, code, blank, code, code, code});
    // ")\"" inside does not end a raw string whose delimiter is "d".
    CHECK(kinds("auto r = R\"d( )\" /* )d\";\n// c\n") == std::vector{code, comment});
    CHECK(kinds("auto r = u8R\"(\n/*\n)\";\n") == std::vector{code, code, code});
    CHECK(kinds("auto r = LR\"(\n//\n)\";\n") == std::vector{code, code, code});
    // An identifier merely ending in R is not a raw string prefix; read as
    // one, the "(" would open a raw string and swallow line 2.
    CHECK(kinds("x = BAR\"(\";\n// real\n") == std::vector{code, comment});
    // A backslash-newline inside a raw string is content, not a splice.
    CHECK(kinds("auto r = R\"(a\\\n// in)\";\n") == std::vector{code, code});
}

TEST_CASE("line splices", "[loc]") {
    // A spliced line comment continues onto the next physical line.
    CHECK(kinds("// c \\\nstill comment\nint a;\n") == std::vector{comment, comment, code});
    CHECK(kinds("#define M(x) \\\n  (x + 1)\n") == std::vector{code, code});
    // The splice backslash itself is not code.
    CHECK(kinds("#define N \\\n\n") == std::vector{code, blank});
    // Splicing happens before comments are recognised: "/\" + "/" is "//".
    CHECK(kinds("int a; /\\\n/ comment\n") == std::vector{code, comment});
    CHECK(kinds("int a; /\\\r\n/ comment\r\n") == std::vector{code, comment});
}

TEST_CASE("a UTF-8 byte order mark is not code", "[loc]") {
    CHECK(kinds("\xEF\xBB\xBF// c\n") == std::vector{comment});
}

TEST_CASE("count_lines totals the classification", "[loc]") {
    const LocCounts counts = count_lines("// c\n\nint x; // t\n/* a\n b */\n");
    CHECK(counts == LocCounts{.physical = 5, .blank = 1, .comment = 3, .code = 1});
}
