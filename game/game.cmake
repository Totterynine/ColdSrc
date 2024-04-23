set(LIBNAME game)

set(src_dir ${PROJECT_ROOT_PATH}/game)

set(sources ${src_dir}/game_app.cpp)
set(headers ${src_dir}/game_globals.h)

add_library(${LIBNAME} SHARED ${sources} ${headers} )
set_property(TARGET game PROPERTY FOLDER "${SLN_FOLDER_PREFIX}ColdSrc")

target_link_libraries(${LIBNAME} PRIVATE SDL3::SDL3)

