cmake_minimum_required (VERSION 3.21.2)

project ("json_asio"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

add_definitions(-DBOOST_ALL_NO_LIB)
set(Boost_USE_STATIC_LIBS OFF)

FIND_PACKAGE(Boost 1.79.0)
message("Found boost at " ${Boost_INCLUDE_DIR})

# must be before `add_subdirectory`
# see https://stackoverflow.com/questions/30250494/ctest-not-detecting-tests
enable_testing()

# Include sub-projects.
add_subdirectory("boost-json")
add_subdirectory("json-asio")
add_subdirectory("json-asio-tests")

install(TARGETS json_asio
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT json_asio_Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT json_asio_Runtime
        NAMELINK_COMPONENT json_asio_Runtime_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT json_asio_Runtime_Development
)
