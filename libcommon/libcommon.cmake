set(LIBNAME libcommon)

set(src_dir ${PROJECT_ROOT_PATH}/libcommon)
set(public_dir ${PROJECT_ROOT_PATH}/public/libcommon)


set(sources 
    ${src_dir}/module_factory.cpp
    ${src_dir}/module_lib.cpp

)
set(headers 
    ${public_dir}/module_factory.h
    ${public_dir}/module_lib.h
)

add_library(${LIBNAME} STATIC ${sources} ${headers} )
set_property(TARGET libcommon PROPERTY FOLDER "ColdSrc")

target_link_libraries(${LIBNAME} PRIVATE SDL3::SDL3)