cmake_minimum_required (VERSION 3.21)

project ("json_asio"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

FIND_PACKAGE(Boost 1.75.0)
message("Found boost at " ${Boost_INCLUDE_DIR} " with libraries at " ${Boost_LIBRARY_DIRS})

# Include sub-projects.
add_subdirectory("json-asio")

install(TARGETS json_asio
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT json_asio_Runtime
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT json_asio_Runtime
        NAMELINK_COMPONENT json_asio_Runtime_Development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT json_asio_Runtime_Development
)
