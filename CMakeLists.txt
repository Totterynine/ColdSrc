cmake_minimum_required(VERSION 3.21)

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY project)

project( coldsrc_game VERSION 0.1 DESCRIPTION "Study project to learn modern Vulkan" LANGUAGES CXX)

set(PROJECT_ROOT_PATH ${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_ROOT_PATH}/public)

include(LibraryList.cmake)

foreach(curlib IN LISTS LIBRARY_LIST) 
    include(${curlib}/${curlib}.cmake)
endforeach()
