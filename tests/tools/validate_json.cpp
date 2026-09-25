// validate_json SCHEMA DOCUMENT: exits 0 if DOCUMENT validates against
// SCHEMA (JSON Schema draft 7), else prints why and exits 1. Test-only: it
// lets the CLI tests validate a report Tezcatl actually wrote, not only the
// hand-built sample the unit tests use.

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

nlohmann::json load(const std::string& path) {
    std::ifstream in{path};
    if (!in) {
        throw std::runtime_error{"cannot open " + path};
    }
    return nlohmann::json::parse(in);
}

int validate(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cerr << "usage: validate_json SCHEMA DOCUMENT\n";
        return EXIT_FAILURE;
    }
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(load(args.at(1)));
    try {
        validator.validate(load(args.at(2)));
    } catch (const std::exception& error) {
        std::cerr << "invalid: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
    std::cout << "valid\n";
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char** argv) {
    // Anything else that throws (an unreadable file, a broken schema) is not
    // a verdict on the document; report it without iostreams, which could
    // throw again.
    try {
        return validate({argv, std::next(argv, argc)});
    } catch (const std::exception& error) {
        (void)std::fputs("validate_json: ", stderr);
        (void)std::fputs(error.what(), stderr);
        (void)std::fputs("\n", stderr);
    } catch (...) {
        (void)std::fputs("validate_json: unknown error\n", stderr);
    }
    return 2;
}
