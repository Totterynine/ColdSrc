cmake_minimum_required(VERSION 3.21)

# Only set the cxx_standard if it is not set by someone else
if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY project)

project( coldsrc_game VERSION 0.1 DESCRIPTION "Study project to learn modern Vulkan" LANGUAGES CXX)

set(PROJECT_ROOT_PATH ${PROJECT_SOURCE_DIR})
set(THIRDPARTY_PATH ${PROJECT_ROOT_PATH}/thirdparty)

add_subdirectory(${THIRDPARTY_PATH}/VulkanMemoryAllocator)
add_subdirectory(${THIRDPARTY_PATH}/vk-bootstrap)
add_subdirectory(${THIRDPARTY_PATH}/glm)
add_subdirectory(${THIRDPARTY_PATH}/SDL)

include_directories(${PROJECT_ROOT_PATH}/public)

link_libraries(glm-header-only)

include(LibraryList.cmake)

foreach(curlib IN LISTS ESSENTIAL_LIST) 
    include(${curlib}/${curlib}.cmake)
endforeach()

foreach(curlib IN LISTS ESSENTIAL_LIST) 
    link_libraries(${curlib})
endforeach()

foreach(curlib IN LISTS LIBRARY_LIST) 
    include(${curlib}/${curlib}.cmake)
endforeach()
