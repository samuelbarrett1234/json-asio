add_library(boost_json "src.cpp")
add_library(Boost::json ALIAS boost_json)

target_include_directories(boost_json PUBLIC ${Boost_INCLUDE_DIR})
