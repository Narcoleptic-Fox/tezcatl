#include "parse/command_line.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using tezcatl::parse::CompileCommand;
using tezcatl::parse::is_cl_driver;
using tezcatl::parse::portable_arguments;

namespace {

// A directory that is absolute on both Windows and Linux.
fs::path project() {
    return fs::current_path().root_path() / "proj";
}

std::string in_project(const char* relative) {
    return (project() / relative).lexically_normal().string();
}

CompileCommand command(std::vector<std::string> arguments) {
    return {.file = (project() / "src" / "a.cpp").lexically_normal(),
            .directory = project(),
            .arguments = std::move(arguments)};
}

} // namespace

TEST_CASE("the MSVC-compatible driver is recognised", "[command_line]") {
    CHECK(is_cl_driver({"cl.exe", "/c", "a.cpp"}));
    CHECK(is_cl_driver({R"(C:\VS\bin\CL.EXE)", "/c", "a.cpp"}));
    CHECK(is_cl_driver({"/usr/bin/clang-cl", "/c", "a.cpp"}));
    CHECK(is_cl_driver({"anything", "--driver-mode=cl"}));
    CHECK_FALSE(is_cl_driver({"clang++", "-c", "a.cpp"}));
    CHECK_FALSE(is_cl_driver({"/usr/bin/gcc", "-c", "a.cpp"}));
    CHECK_FALSE(is_cl_driver({}));
}

TEST_CASE("GCC-style include paths are made absolute; the source file moves last",
          "[command_line]") {
    const auto arguments =
        portable_arguments(command({"g++", "-Iinc", "-I", "gen", "-isystem", "third",
                                    "--sysroot=sys", "-DX=1", "-c", "src/a.cpp", "-o", "out/a.o"}),
                           {"-Wno-error"});
    const std::vector<std::string> expected{"g++",
                                            "-working-directory=" + project().string(),
                                            "-I" + in_project("inc"),
                                            "-I",
                                            in_project("gen"),
                                            "-isystem",
                                            in_project("third"),
                                            "--sysroot=" + in_project("sys"),
                                            "-DX=1",
                                            "-c",
                                            "-o",
                                            "out/a.o", // an output, left alone
                                            "-Wno-error",
                                            "--",
                                            in_project("src/a.cpp")};
    CHECK(arguments == expected);
}

TEST_CASE("cl-style include paths are made absolute and outputs are not", "[command_line]") {
    const auto arguments = portable_arguments(
        command({"cl.exe", "/Iinc", "-Igen", "/I", "third", "/external:Iext", "/FIpch.h",
                 "/Foout\a.obj", "/Fdout\a.pdb", "/WX", "/c", "src/a.cpp"}),
        {"-Wno-error"});
    // No working-directory flag: clang-cl has no dependable one.
    const std::vector<std::string> expected{"cl.exe",
                                            "/I" + in_project("inc"),
                                            "-I" + in_project("gen"),
                                            "/I",
                                            in_project("third"),
                                            "/external:I" + in_project("ext"),
                                            "/FI" + in_project("pch.h"),
                                            "/Foout\a.obj",
                                            "/Fdout\a.pdb",
                                            "/WX",
                                            "/c",
                                            "-Wno-error", // after /WX, so it wins
                                            "/Tp",
                                            in_project("src/a.cpp")};
    CHECK(arguments == expected);
}

TEST_CASE("absolute paths are left exactly as written", "[command_line]") {
    const std::string absolute_include = (project() / "abs").string();
    const auto arguments = portable_arguments(
        command({"clang++", "-I" + absolute_include, "-c", in_project("src/a.cpp")}), {});
    const std::vector<std::string> expected{"clang++",
                                            "-working-directory=" + project().string(),
                                            "-I" + absolute_include,
                                            "-c",
                                            "--",
                                            in_project("src/a.cpp")};
    CHECK(arguments == expected);
}

TEST_CASE("clang-cl names the source file with its language", "[command_line]") {
    const auto source_marker = [](std::vector<std::string> arguments, const char* file) {
        CompileCommand c{.file = project() / file, .directory = project(), .arguments = {}};
        arguments.emplace_back(file);
        c.arguments = std::move(arguments);
        const auto result = portable_arguments(c, {});
        return result.at(result.size() - 2);
    };
    CHECK(source_marker({"cl.exe", "/c"}, "a.cpp") == "/Tp");
    CHECK(source_marker({"cl.exe", "/c"}, "a.c") == "/Tc");
    // The project's own /TP or /TC decides, as it does in the real build.
    CHECK(source_marker({"cl.exe", "/TP", "/c"}, "a.c") == "/Tp");
    CHECK(source_marker({"cl.exe", "-TC", "/c"}, "a.cpp") == "/Tc");
}
