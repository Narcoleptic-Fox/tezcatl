# Per-target settings for Tezcatl's own code. Applied per target rather than
# globally so third-party code (CLI11, Catch2) is not held to our warning set.

function(tezcatl_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8
                                                 $<$<BOOL:${TEZCATL_WERROR}>:/WX>)
    else()
        target_compile_options(
            ${target}
            PRIVATE -Wall
                    -Wextra
                    -Wpedantic
                    -Wshadow
                    -Wconversion
                    -Wsign-conversion
                    $<$<BOOL:${TEZCATL_WERROR}>:-Werror>)
    endif()
endfunction()

# Windows has no rpath: copy imported DLLs (libclang.dll) next to the binary so
# it runs from the build tree. A no-op elsewhere.
function(tezcatl_copy_runtime_dlls target)
    if(WIN32)
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${target}>
                    $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS)
    endif()
endfunction()
