# TODO: change compiler to g++-15? (Or fix clang 'algorithm' file not found)
cmake_minimum_required(VERSION 3.16)
project(exchange LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put all executables in build/bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# === Core library ===
# add_library(exchange STATIC
#     exchange/order.cpp
#     exchange/book.cpp
# )
# target_include_directories(exchange PUBLIC ${CMAKE_SOURCE_DIR}/exchange)

# === Server executable ===
# add_executable(exchange_server server/main.cpp)
# target_link_libraries(exchange_server PRIVATE exchange)

# === Client executable ===
add_executable(exchange_client client/main.cpp)
# target_link_libraries(exchange_client PRIVATE exchange)

# === Tests ===
# enable_testing()
# add_executable(unit_tests tests/unit_tests.cpp)
# target_link_libraries(unit_tests PRIVATE exchange)
# add_test(NAME all_tests COMMAND unit_tests)
