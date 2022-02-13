add_library(Exchanger STATIC "Exchanger.cpp" "Exchanger.h")

target_link_libraries(Exchanger "Util" "asio::asio" "Indexing")

target_include_directories(Exchanger PRIVATE "${PROJECT_SOURCE_DIR}/Util" "${PROJECT_SOURCE_DIR}/Indexing" "${PROJECT_SOURCE_DIR}/asio")