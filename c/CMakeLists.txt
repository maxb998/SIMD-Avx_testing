cmake_minimum_required(VERSION 3.0.0)
project(kCentersOutliers VERSION 0.1.0)

include(CTest)
enable_testing()

add_definitions( "-mavx2" )

add_subdirectory(lib)
add_subdirectory(test)
target_include_directories(kCentersOutliersLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/lib")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
