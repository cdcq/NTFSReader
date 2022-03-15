cmake_minimum_required(VERSION 3.20)
project(NTFSReader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        REQUIRED)

add_executable(NTFSReader main.cpp controller.cpp controller.h layers/image/e01.cpp layers/image/e01.h layers/image/interface.h layers/image/interface.cpp layers/image/image.cpp layers/image/image.h layers/image/dd.cpp layers/image/dd.h layers/disk/interface.cpp layers/disk/interface.h layers/disk/disk.cpp layers/disk/disk.h layers/disk/guid.cpp layers/disk/guid.h layers/filesystem/interface.cpp layers/filesystem/interface.h layers/filesystem/ntfs.cpp layers/filesystem/ntfs.h layers/filesystem/filesystem.cpp layers/filesystem/filesystem.h layers/disk/logic.cpp layers/disk/logic.h)
target_link_libraries(NTFSReader
        Qt5::Core
        )

