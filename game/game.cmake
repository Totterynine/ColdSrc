set(LIBNAME game)

set(src_dir ${PROJECT_ROOT_PATH}/game)

set(sources ${src_dir}/game_app.cpp)
set(headers )

add_library(${LIBNAME} SHARED ${sources} ${headers} )

