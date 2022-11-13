add_library(json_asio INTERFACE)

target_include_directories(json_asio INTERFACE .)

target_include_directories(json_asio INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(json_asio INTERFACE Boost::json)
