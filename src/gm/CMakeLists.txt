cmake_minimum_required(VERSION 3.0.0)
project(gm VERSION 0.1.0)

include(CTest)
enable_testing()



add_library(gm gm.cpp signature.cpp convert.cpp p256.cpp sm3.cpp)



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
