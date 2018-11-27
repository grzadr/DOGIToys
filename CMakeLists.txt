cmake_minimum_required(VERSION 3.10)
project(DOGIToys VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(hkl)

find_package (Python3 COMPONENTS Interpreter Development)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()

if (NOT PYTHON_INSTALL_PREFIX)
    set(PYTHON_INSTALL_PREFIX ${Python3_SITELIB} CACHE PATH "..." FORCE)
endif()
if (NOT HEADER_INSTALL_PREFIX)
    set(HEADER_INSTALL_PREFIX "/usr/local" CACHE PATH "..." FORCE)
endif()

set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Sql REQUIRED)

add_library(DOGIToys SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/connect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/initiate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/populate/populate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/populate/genomic_features.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/populate/mapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/populate/gene_ontology.cpp
    )

target_include_directories(DOGIToys
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/agizmo/include
)

target_compile_features(DOGIToys PRIVATE cxx_std_17)
target_compile_options(DOGIToys PRIVATE
    -Werror -Wall
    -pthread -O3
    -mtune=native -march=x86-64
    -pipe -fPIE -fPIC)

target_link_libraries(DOGIToys Qt5::Core Qt5::Sql)


pybind11_add_module(pyDOGIToys ${CMAKE_CURRENT_SOURCE_DIR}/src/pydogitoys.cpp)
target_include_directories(pyDOGIToys
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/include
        ${CMAKE_CURRENT_SOURCE_DIR}/hkl/agizmo/include
)
set(PYBIND11_CPP_STANDARD -std=c++17)
# target_compile_options(pyDOGIToys PRIVATE
#     -Werror -Wall
#     -pthread -O3
#     -mtune=native -march=x86-64
#     -pipe -fPIE -fPIC)
target_link_libraries(pyDOGIToys PRIVATE DOGIToys)

install(DIRECTORY include/dogitoys DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(TARGETS DOGIToys
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(TARGETS pyDOGIToys
        LIBRARY DESTINATION ${PYTHON_INSTALL_PREFIX})

# add_custom_command(TARGET pyDOGIToys POST_BUILD
#     COMMAND
#         "python3"
#         "${CMAKE_CURRENT_SOURCE_DIR}/test/test.py"
#         "${CMAKE_CURRENT_SOURCE_DIR}/test/input"
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
