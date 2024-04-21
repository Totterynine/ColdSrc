set(LIBNAME rendersystem)

set(src_dir ${PROJECT_ROOT_PATH}/rendersystem)

set(sources ${src_dir}/rendersystem.cpp)
set(headers )

add_library(${LIBNAME} SHARED ${sources} ${headers} )

target_link_libraries(${LIBNAME} PRIVATE vk-bootstrap::vk-bootstrap GPUOpen::VulkanMemoryAllocator)