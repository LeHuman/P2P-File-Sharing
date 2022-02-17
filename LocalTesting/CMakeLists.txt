add_executable (LocalTesting "LocalTesting.cpp" )

target_link_libraries(LocalTesting "Util" "Indexing" "Console" "Exchanger" "Peer")