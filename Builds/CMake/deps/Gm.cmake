include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(src/gm)

add_library(SmCrypto::gm ALIAS gm)


find_package(GMP REQUIRED)

add_library (gmp_lib STATIC IMPORTED GLOBAL)

set_target_properties (gmp_lib PROPERTIES
IMPORTED_LOCATION_DEBUG
  ${GMP_LIBRARY}
IMPORTED_LOCATION_RELEASE
  ${GMP_LIBRARY}
INTERFACE_INCLUDE_DIRECTORIES
  ${GMP_INCLUDE_DIR})
 

target_link_libraries(gm PRIVATE gmp_lib)

target_link_libraries(ripple_libs INTERFACE SmCrypto::gm)

install(FILES src/gm/gm.h DESTINATION include/gm)