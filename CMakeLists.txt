cmake_minimum_required(VERSION 3.16)
project(CryptoCommApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED) 

find_path(PBC_INCLUDE_DIR NAMES pbc/pbc.h PATH_SUFFIXES include)
find_library(PBC_LIBRARY NAMES pbc PATH_SUFFIXES lib lib64)
find_library(GMP_LIBRARY NAMES gmp PATH_SUFFIXES lib lib64)

if(NOT PBC_INCLUDE_DIR OR NOT PBC_LIBRARY)
    message(FATAL_ERROR "PBC 库未找到！请执行: sudo apt install libpbc-dev")
endif()
if(NOT GMP_LIBRARY)
    message(FATAL_ERROR "GMP 库未找到！请执行: sudo apt install libgmp-dev")
endif()


add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/MainWindow.cpp 
    src/MainWindow.h
    src/CryptoManager.cpp 
    src/CryptoManager.h
    src/NetworkManager.cpp
    src/NetworkManager.h
    src/models.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PBC_INCLUDE_DIR}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    ${PBC_LIBRARY}
    ${GMP_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
)


add_executable(CryptoServer 
    src/Server_KGC.cpp
)

target_include_directories(CryptoServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PBC_INCLUDE_DIR}
)


target_link_libraries(CryptoServer PRIVATE
    ${PBC_LIBRARY}
    ${GMP_LIBRARY}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads 
)