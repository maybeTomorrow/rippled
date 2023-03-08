include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(src/gm)

target_link_libraries(ripple_libs INTERFACE gm)



find_package(GMP REQUIRED)

include_directories(${GMP_INCLUDE_DIR})
link_directories(${GMP_LIBRARY_DIR})


target_link_libraries(ripple_libs INTERFACE gmp gmpxx)


install(FILES src/gm/gm.h DESTINATION include/gm)