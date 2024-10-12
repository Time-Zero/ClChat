aux_source_directory(. SRC_LIST)
add_executable(json_test ${SRC_LIST})
find_package(nlohmann_json 3.6.0 REQUIRED)
target_link_libraries(json_test nlohmann_json::nlohmann_json)
