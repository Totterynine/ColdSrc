set(LIBNAME rendersystem)

set(src_dir ${PROJECT_ROOT_PATH}/rendersystem)

set(sources ${src_dir}/rendersystem.cpp)
set(headers )

add_library(${LIBNAME} SHARED ${sources} ${headers} )