function(target_set_warnings)
    set(OneValue TARGET)
    cmake_parse_arguments(
        target_set_warnings
        "${options}"
        "${OneValue}"
        "${MultiValue}"
        ${ARGN}
    )

    if(${CMAKE_CXX_COMPILER_ID} MATCHES "MSVC")
        set(WARNING_OPTIONS /W4 /permissive- /WX)
    elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "CLANG")
        set(WARNING_OPTIONS -Wall -Wextra -Wpendatic -Werror)
    else()
        message(STATUS "target_set_warnings function is only supported msvc and gcc")
        return()
    endif()

    target_compile_options(${TARGET} PRIVATE ${WARNING_OPTIONS})
endfunction()

