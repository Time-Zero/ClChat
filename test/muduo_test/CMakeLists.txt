aux_source_directory(. SRC_LIST)
add_executable(muduo_test ${SRC_LIST})
target_link_libraries(muduo_test muduo_net muduo_base pthread)