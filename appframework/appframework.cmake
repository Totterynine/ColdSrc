set(LIBNAME app)

set(src_dir ${PROJECT_ROOT_PATH}/appframework)

set(sources ${src_dir}/appframework.cpp ${src_dir}/entry.cpp)
set(headers ${src_dir}/appframework.h)

add_executable(${LIBNAME} ${sources} ${headers} )