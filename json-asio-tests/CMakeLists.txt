add_executable(
    json_asio_tests
    "main.cpp" "json_asio_tests.cpp"
)

message("Boost libraries are " ${Boost_LIBRARIES})
target_include_directories(json_asio_tests PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(json_asio_tests PUBLIC json_asio)
add_test(NAME json_asio_tests COMMAND  $<TARGET_FILE:json_asio_tests>)
