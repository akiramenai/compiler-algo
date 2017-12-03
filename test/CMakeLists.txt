include(ExternalProject)

set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
)

add_executable(unittest
  test.cpp)

add_dependencies(unittest googletest)

set(GTEST_LIB_DIR ${EXTERNAL_INSTALL_LOCATION}/lib)

target_include_directories(unittest
  PUBLIC
  ${EXTERNAL_INSTALL_LOCATION}/include)

add_library(GTEST_LIB IMPORTED STATIC)
add_library(GTEST_MAIN_LIB IMPORTED STATIC)

add_dependencies(GTEST_LIB googletest)
add_dependencies(GTEST_MAIN_LIB googletest)

if (APPLE)
	set_target_properties(GTEST_LIB PROPERTIES IMPORTED_LOCATION ${EXTERNAL_INSTALL_LOCATION}/lib/libgtest.a)
	set_target_properties(GTEST_MAIN_LIB PROPERTIES IMPORTED_LOCATION ${EXTERNAL_INSTALL_LOCATION}/lib/libgtest_main.a)
endif (APPLE)

target_link_libraries(unittest GTEST_LIB GTEST_MAIN_LIB pthread)
