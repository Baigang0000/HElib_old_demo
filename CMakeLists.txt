cmake_minimum_required(VERSION 3.10)
project(bch_ckks_experiment CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(helib REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/bch)

add_executable(bch_ckks_experiment main.cpp bch/bch.cpp)
target_link_libraries(bch_ckks_experiment helib)
