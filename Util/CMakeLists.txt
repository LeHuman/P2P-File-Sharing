add_library(Util STATIC "Log.h" "Log.cpp" "Folder.h" "Folder.cpp" )

target_link_libraries(Util "PicoSHA2")

target_include_directories(Util PUBLIC ".")