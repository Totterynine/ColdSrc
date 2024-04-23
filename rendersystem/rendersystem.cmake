set(LIBNAME rendersystem)

set(src_dir ${PROJECT_ROOT_PATH}/rendersystem)
set(public_dir ${PROJECT_ROOT_PATH}/public/rendersystem)

set(sources ${src_dir}/rendersystem.cpp)
set(headers ${src_dir}/rendersystem.h ${public_dir}/irendersystem.h)

add_library(${LIBNAME} SHARED ${sources} ${headers} )

set_property(TARGET rendersystem PROPERTY FOLDER "${SLN_FOLDER_PREFIX}ColdSrc")

target_link_libraries(${LIBNAME} PRIVATE vk-bootstrap::vk-bootstrap GPUOpen::VulkanMemoryAllocator Vulkan::Vulkan)