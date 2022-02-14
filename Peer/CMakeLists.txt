add_executable(Peer "Peer.cpp" )

target_link_libraries(Peer "Util" "Indexing" "Console" "Exchanger")

target_include_directories(Peer PRIVATE "${PROJECT_SOURCE_DIR}/Util" "${PROJECT_SOURCE_DIR}/Indexing" "${PROJECT_SOURCE_DIR}/Console" "${PROJECT_SOURCE_DIR}/Exchanger")