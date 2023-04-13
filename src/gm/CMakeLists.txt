cmake_minimum_required(VERSION 3.0.0)
project(gm VERSION 0.1.0)

include(CTest)
enable_testing()




add_library(gm STATIC gm.cpp signature.cpp convert.cpp p256.cpp sm3.cpp)

add_library(SmCrypto::gm ALIAS gm)


find_package(GMP REQUIRED)

# add_library (gmp::libgmp STATIC IMPORTED GLOBAL)

target_link_libraries(gm PRIVATE gmp::libgmp)
 


install(
  TARGETS gm
  EXPORT ${PROJECT_NAME}-exports
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
install(
  EXPORT ${PROJECT_NAME}-exports
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
)
install(
  FILES gm.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
