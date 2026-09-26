#include "coverage/readers.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace tezcatl::coverage {

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

[[noreturn]] void fail(const fs::path& source, std::string_view problem) {
    throw std::runtime_error(source.generic_string() + ": " + std::string{problem});
}

// One document, or one per line as `gcov --stdout` writes them.
std::vector<json> documents(std::string_view text, const fs::path& source) {
    if (json::accept(text)) {
        return {json::parse(text)};
    }
    std::vector<json> result;
    while (!text.empty()) {
        const std::size_t end = text.find('\n');
        const std::string_view line = text.substr(0, end);
        text.remove_prefix(end == std::string_view::npos ? text.size() : end + 1);
        if (line.find_first_not_of(" \t\r") == std::string_view::npos) {
            continue;
        }
        if (!json::accept(line)) {
            fail(source, "not valid JSON, as one document or one per line");
        }
        result.push_back(json::parse(line));
    }
    return result;
}

FileRecord& record_for_details(CoverageData& data, const fs::path& file, const fs::path& source) {
    FileRecord& record = data[file];
    if (record.has_totals()) {
        fail(source, "line data for " + file.string() + ", which also has llvm-cov totals");
    }
    return record;
}

// How a branch is told apart from the line's others when units are merged,
// as gcovr does. gcov's JSON format 2 (GCC 14) names each branch by the
// blocks it leaves and enters, so the same branch lines up across the units
// and template instances that compile the line, and different code compiled
// on one line (a macro expanded in two functions) stays apart. Format 1
// (GCC 13 and earlier) has no block ids, and only the branch's position on
// the line is left.
std::string branch_id(const json& branch, std::size_t index_on_line) {
    if (branch.contains("source_block_id") && branch.contains("destination_block_id")) {
        return "block " + std::to_string(branch.at("source_block_id").get<std::uint64_t>()) + '>' +
               std::to_string(branch.at("destination_block_id").get<std::uint64_t>());
    }
    return std::to_string(index_on_line);
}

unsigned line_number(const json& value, const fs::path& source) {
    const auto line = value.get<std::uint64_t>();
    if (line == 0 || line > std::numeric_limits<unsigned>::max()) {
        fail(source, "line number out of range: " + std::to_string(line));
    }
    return static_cast<unsigned>(line);
}

void read_gcov_document(const json& document, const fs::path& source, CoverageData& data) {
    const fs::path directory{document.value("current_working_directory", std::string{})};
    for (const json& file : document.at("files")) {
        const fs::path name{file.at("file").get<std::string>()};
        FileRecord& record =
            record_for_details(data, resolve_recorded_path(name, directory), source);
        for (const json& function : file.at("functions")) {
            const std::string display = function.contains("demangled_name")
                                            ? function.at("demangled_name").get<std::string>()
                                            : function.at("name").get<std::string>();
            record.add_function(display, function.at("execution_count").get<std::uint64_t>());
        }
        for (const json& line : file.at("lines")) {
            const unsigned number = line_number(line.at("line_number"), source);
            record.add_line(number, line.at("count").get<std::uint64_t>());
            std::size_t index_on_line = 0;
            for (const json& branch : line.value("branches", json::array())) {
                record.add_branch(number, branch_id(branch, index_on_line++),
                                  branch.at("count").get<std::uint64_t>());
            }
        }
    }
}

Counts llvm_totals(const json& summary) {
    const auto count = [&summary](const char* kind, const char* field) {
        return summary.at(kind).at(field).get<std::size_t>();
    };
    return {.lines = count("lines", "count"),
            .lines_covered = count("lines", "covered"),
            .branches = count("branches", "count"),
            .branches_covered = count("branches", "covered"),
            .functions = count("functions", "count"),
            .functions_covered = count("functions", "covered")};
}

} // namespace

void read_gcov_json(std::string_view text, const fs::path& source, CoverageData& data) {
    try {
        for (const json& document : documents(text, source)) {
            read_gcov_document(document, source, data);
        }
    } catch (const json::exception& error) {
        fail(source, std::string{"unexpected gcov JSON: "} + error.what());
    }
}

void read_llvm_json(std::string_view text, const fs::path& source, CoverageData& data) {
    try {
        const json document = json::parse(text);
        for (const json& export_data : document.at("data")) {
            for (const json& file : export_data.at("files")) {
                const fs::path name =
                    fs::path{file.at("filename").get<std::string>()}.lexically_normal();
                FileRecord& record = data[name];
                if (record.has_totals() || record.has_details()) {
                    fail(source, name.string() +
                                     " is reported twice; merge the profiles with llvm-profdata "
                                     "and export once");
                }
                record.add_totals(llvm_totals(file.at("summary")));
            }
        }
    } catch (const json::exception& error) {
        fail(source, std::string{"unexpected llvm-cov JSON: "} + error.what());
    }
}

} // namespace tezcatl::coverage
