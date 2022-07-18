 
add_test(NAME sha256_once
        COMMAND lua ${CMAKE_CURRENT_SOURCE_DIR}/sha256_once.lua
        WORKING_DIRECTORY $<TARGET_FILE_DIR:infra>)

add_test(NAME strerror
        COMMAND lua ${CMAKE_CURRENT_SOURCE_DIR}/strerror.lua
        WORKING_DIRECTORY $<TARGET_FILE_DIR:infra>)
