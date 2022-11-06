add_library(
    json_asio
)

target_include_directories(json_asio PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(json_asio PRIVATE ${Boost_LIBRARIES})
