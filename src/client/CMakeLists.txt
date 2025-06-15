
aux_source_directory(. CLIENT_SRC_LIST)

add_executable(ChatClient ${CLIENT_SRC_LIST})

target_link_libraries(ChatClient pthread)