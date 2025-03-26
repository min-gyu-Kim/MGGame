function(target_set_ipo)
    set(OneValue TARGET)
    cmake_parse_arguments(
        target_set_warnings
        "${options}"
        "${OneValue}"
        "${MultiValue}"
        ${ARGN}
    )

    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set_property(TARGET ${TARGET} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO is not supported: ${output}")
    endif()
endfunction(target_set_ipo)
