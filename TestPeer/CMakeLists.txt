add_executable(TestPeer "TestPeer.cpp"  "TestStrings.h")

target_link_libraries(TestPeer "Indexing" "Exchanger" "Peer" "Util" "tclap")
