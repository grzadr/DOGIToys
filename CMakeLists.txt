cmake_minimum_required(VERSION 3.10)
project(DOGIToys VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/lib/python3.6" CACHE PATH "..." FORCE)
endif()

if (NOT HEADER_INSTALL_PREFIX)
    set(HEADER_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Sql REQUIRED)

add_library(DOGIToys SHARED
${CMAKE_CURRENT_SOURCE_DIR}/src/dogitoys.cpp 
${CMAKE_CURRENT_SOURCE_DIR}/src/dogitoys-populate.cpp)

target_include_directories(DOGIToys
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/agizmo/include
)

target_compile_features(DOGIToys PRIVATE cxx_std_17)

target_link_libraries(DOGIToys Qt5::Core Qt5::Sql)

add_subdirectory(hkl/agizmo/pybind11)

install(TARGETS DOGIToys LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY include/dogitoys DESTINATION ${HEADER_INSTALL_PREFIX}/include)
