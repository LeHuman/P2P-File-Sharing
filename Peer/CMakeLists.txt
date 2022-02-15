add_executable(Peer "Peer.cpp" )

target_link_libraries(Peer "Util" "Indexing" "Console" "Exchanger")