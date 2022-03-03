add_library(Util STATIC "Log.h" "Log.cpp" "Folder.h" "Folder.cpp" "Config.h" "Config.cpp")

target_link_libraries(Util "cryptopp-static" "nlohmann_json" "Indexing")

target_include_directories(Util PUBLIC ".")