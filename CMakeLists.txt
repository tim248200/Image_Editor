cmake_minimum_required(VERSION 4.2)
project(ImageEditor)

set(CMAKE_CXX_STANDARD 20)

add_executable(ImageEditor main.cpp)

target_link_libraries(ImageEditor PRIVATE comdlg32 gdiplus)