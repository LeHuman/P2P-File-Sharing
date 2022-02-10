add_executable(Server "Server.cpp" )

target_link_libraries(Server "Console" "Indexing" "Util")

target_include_directories(Server PRIVATE "${PROJECT_SOURCE_DIR}/Util" "${PROJECT_SOURCE_DIR}/Indexing" "${PROJECT_SOURCE_DIR}/Console")