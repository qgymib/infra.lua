
add_executable(infra_loader loader.c)

target_link_libraries(infra_loader
    PRIVATE
        infra
        ${LUA_LIBRARY})

setup_target_wall(infra_loader)
set(LUA_EXE infra_loader)

set(CASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/case)
