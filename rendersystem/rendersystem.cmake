set(LIBNAME rendersystem)

set(src_dir ${PROJECT_ROOT_PATH}/rendersystem)
set(public_dir ${PROJECT_ROOT_PATH}/public/rendersystem)

set(sources ${src_dir}/rendersystem.cpp ${src_dir}/utils.cpp ${src_dir}/shader.cpp ${src_dir}/rendertarget.cpp ${src_dir}/descriptorsets.cpp)
set(headers ${src_dir}/rendersystem.h ${src_dir}/utils.h ${src_dir}/shader.h ${src_dir}/rendertarget.h ${src_dir}/descriptorsets.h ${src_dir}/vulkan_common.h ${public_dir}/irendersystem.h ${public_dir}/ishader.h ${public_dir}/rendersystem_types.h)

add_library(${LIBNAME} SHARED ${sources} ${headers} )

set_property(TARGET rendersystem PROPERTY FOLDER "${SLN_FOLDER_PREFIX}ColdSrc")

target_link_libraries(${LIBNAME} PRIVATE vk-bootstrap::vk-bootstrap GPUOpen::VulkanMemoryAllocator Vulkan::Vulkan)