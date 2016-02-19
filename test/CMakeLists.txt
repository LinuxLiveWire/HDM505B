find_package(Threads REQUIRED)

include(ExternalProject)

ExternalProject_Add(
        googletest
        URL ${CMAKE_SOURCE_DIR}/extern/gtest-1.7.zip
        URL_MD5 b8c9babd44aeeb68d2f5bd48a2923ea5
        PREFIX ${CMAKE_BINARY_DIR}/googletest
        # Disable install step
        INSTALL_COMMAND ""
        LOG_CONFIGURE ON
        LOG_BUILD ON
)

ExternalProject_Get_Property(googletest binary_dir source_dir)

add_library(gtest      STATIC IMPORTED GLOBAL)
add_library(gmock      STATIC IMPORTED GLOBAL)
add_library(gtest_main STATIC IMPORTED GLOBAL)

set_target_properties(gtest PROPERTIES
        IMPORTED_LOCATION ${binary_dir}/googlemock/gtest/libgtest.a
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
        )
set_target_properties(gmock PROPERTIES
        IMPORTED_LOCATION ${binary_dir}/googlemock/libgmock.a
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
        )
set_target_properties(gtest_main PROPERTIES
        IMPORTED_LOCATION ${binary_dir}/googlemock/gtest/libgtest_main.a
        IMPORTED_LINK_INTERFACE_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}"
        )

add_dependencies(gtest      googletest)
add_dependencies(gmock      googletest)
add_dependencies(gtest_main googletest)

file(GLOB SRCS *.cpp)

ADD_EXECUTABLE(test_HDM50 ${SRCS} ../HDM50.cpp)

include_directories(
        ${source_dir}/googlemock/include
        ${source_dir}/googletest/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(test_HDM50
        gtest
        gmock
        gtest_main
        Qt5::Core
)

add_test(NAME compass_test COMMAND test_HDM50)
