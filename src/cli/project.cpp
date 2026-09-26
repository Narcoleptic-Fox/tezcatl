#include "cli/project.hpp"

#include "parse/compilation_database.hpp"
#include "scan/paths.hpp"
#include "scan/source_files.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace tezcatl::cli {

namespace fs = std::filesystem;

namespace {

// Enough to diagnose a broken unit without burying the rest of the output.
constexpr std::size_t max_errors_shown_per_unit = 5;

void report_errors(const parse::ParsedUnit& parsed, std::ostream& err) {
    std::size_t shown = 0;
    for (const parse::ParseError& error : parsed.errors) {
        if (shown++ == max_errors_shown_per_unit) {
            err << "  ... and " << parsed.errors.size() - max_errors_shown_per_unit
                << " more errors in " << parsed.file.generic_string() << '\n';
            break;
        }
        err << "error: " << error.message << '\n';
    }
}

// Threads for `jobs`: as asked, or one per hardware thread. The standard
// allows hardware_concurrency() to return 0 when it cannot tell.
std::size_t thread_count(unsigned jobs) {
    if (jobs > 0) {
        return jobs;
    }
    return std::max(1U, std::thread::hardware_concurrency());
}

} // namespace

Project::Project(const ProjectOptions& options)
    : options_(options), root_(fs::absolute(options.root).lexically_normal()),
      build_(options.build_directory.empty()
                 ? fs::path{}
                 : fs::absolute(options.build_directory).lexically_normal()),
      in_project_([this](const fs::path& file) {
          return scan::is_within(file, root_) &&
                 (excluded_.empty() || !scan::is_within(file, excluded_));
      }),
      naming_(options.root,
              options.module_map.empty() ? config::ModuleMap{}
                                         : config::ModuleMap::load(options.module_map),
              options.test_globs.empty() ? config::FileRoles{}
                                         : config::FileRoles{options.test_globs}) {
    // A build directory inside the root holds generated code and fetched
    // dependencies. One that is the root, or above it, is an in-source build
    // (Make with bear writes compile_commands.json at the top): excluding it
    // would exclude the whole project.
    if (!build_.empty() && scan::is_within(build_, root_) && !scan::is_within(root_, build_)) {
        excluded_ = build_;
    }
}

ScanPlan Project::plan() const {
    ScanPlan plan;
    for (parse::CompileCommand& command : parse::load_compilation_database(build_)) {
        if (!scan::is_source_file(command.file)) {
            ++plan.skipped;
            continue;
        }
        // Its functions, declarations and includes would all be filtered out:
        // parsing it would only cost time (half of a run on Tezcatl itself).
        if (!excluded_.empty() && scan::is_within(command.file, excluded_)) {
            ++plan.in_build_directory;
            continue;
        }
        plan.units.push_back(std::move(command));
    }
    return plan;
}

ScanTotals Project::scan(const ScanPlan& scan_plan,
                         const std::function<void(std::size_t, const parse::ParsedUnit&)>& visit,
                         std::ostream& err) const {
    const std::size_t count = scan_plan.units.size();
    // Per unit, written only by the thread that parsed it.
    std::vector<std::string> errors(count);
    std::vector<unsigned char> failed(count, 0); // not vector<bool>: its elements share bytes
    std::atomic<std::size_t> next{0};

    const std::size_t threads =
        std::max<std::size_t>(1, std::min(count, thread_count(options_.jobs)));
    std::vector<std::exception_ptr> exceptions(threads);
    const auto work = [&](std::size_t worker) {
        try {
            const parse::Parser parser{options_.resource_directory};
            for (std::size_t unit = next++; unit < count; unit = next++) {
                const parse::ParsedUnit parsed = parser.parse(scan_plan.units.at(unit));
                if (!parsed.errors.empty()) {
                    failed.at(unit) = 1;
                    std::ostringstream text;
                    report_errors(parsed, text);
                    errors.at(unit) = std::move(text).str();
                }
                if (parsed.unit) {
                    visit(unit, parsed);
                }
            }
        } catch (...) {
            exceptions.at(worker) = std::current_exception();
            next = count; // the others stop after their current unit
        }
    };
    {
        std::vector<std::jthread> pool;
        pool.reserve(threads);
        for (std::size_t worker = 0; worker < threads; ++worker) {
            pool.emplace_back(work, worker);
        }
    } // joined here
    for (const std::exception_ptr& exception : exceptions) {
        if (exception) {
            std::rethrow_exception(exception);
        }
    }

    ScanTotals totals{.units = count,
                      .units_with_errors = 0,
                      .skipped = scan_plan.skipped,
                      .in_build_directory = scan_plan.in_build_directory};
    for (std::size_t unit = 0; unit < count; ++unit) {
        totals.units_with_errors += failed.at(unit);
        err << errors.at(unit);
    }
    return totals;
}

int Project::finish(const ScanTotals& totals, std::string_view detail, std::ostream& err) const {
    err << "tezcatl: parsed " << totals.units << " translation units, " << totals.units_with_errors
        << " with errors; ";
    if (totals.skipped > 0) {
        err << "skipped " << totals.skipped << " entries that are not C or C++; ";
    }
    if (totals.in_build_directory > 0) {
        err << "skipped " << totals.in_build_directory << " units under the build directory; ";
    }
    err << detail << '\n';
    return (totals.units_with_errors > 0 && !options_.allow_parse_errors) ? 1 : 0;
}

} // namespace tezcatl::cli
