﻿add_executable(Client "Client.cpp" )

target_link_libraries(Client "Peer" "tclap")
