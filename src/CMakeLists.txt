configure_file(tezcatl/version.hpp.in "${PROJECT_BINARY_DIR}/generated/tezcatl/version.hpp" @ONLY)

add_library(
    tezcatl_core STATIC
    cli/coverage_command.cpp
    cli/docs_command.cpp
    cli/functions_command.cpp
    cli/includes_command.cpp
    cli/loc_command.cpp
    cli/project.cpp
    config/file_roles.cpp
    config/modules.cpp
    coverage/json_readers.cpp
    coverage/lcov.cpp
    coverage/model.cpp
    coverage/path_map.cpp
    coverage/read_file.cpp
    graph/digraph.cpp
    metrics/complexity.cpp
    metrics/halstead.cpp
    metrics/loc.cpp
    metrics/summary.cpp
    parse/api.cpp
    parse/command_line.cpp
    parse/compilation_database.cpp
    parse/cursors.cpp
    parse/function_tokens.cpp
    parse/functions.cpp
    parse/includes.cpp
    parse/libclang_info.cpp
    parse/tokens.cpp
    parse/translation_unit.cpp
    report/csv.cpp
    report/include_graph.cpp
    report/json_report.cpp
    report/module_summary.cpp
    report/naming.cpp
    report/tables.cpp
    scan/glob.cpp
    scan/paths.cpp
    scan/source_files.cpp)
add_library(tezcatl::core ALIAS tezcatl_core)
target_include_directories(tezcatl_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                               "${PROJECT_BINARY_DIR}/generated")
target_compile_features(tezcatl_core PUBLIC cxx_std_20)
target_link_libraries(tezcatl_core PUBLIC Libclang::Libclang PRIVATE nlohmann_json::nlohmann_json)
target_compile_definitions(tezcatl_core
                           PRIVATE TEZCATL_CLANG_RESOURCE_DIR="${Libclang_RESOURCE_DIR}")
tezcatl_apply_warnings(tezcatl_core)

add_executable(tezcatl cli/main.cpp)
target_link_libraries(tezcatl PRIVATE tezcatl::core CLI11::CLI11)
tezcatl_apply_warnings(tezcatl)
tezcatl_copy_runtime_dlls(tezcatl)
