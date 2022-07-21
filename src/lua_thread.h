#ifndef __INFRA_API_THREAD_H__
#define __INFRA_API_THREAD_H__

#include "lua_api.h"

#define INFRA_LUA_API_THREAD_CREATE(XX)                                                 \
XX(                                                                                     \
"thread_create", infra_thread_create, NULL,                                             \
"Execution in another native thread and callback in this VM.",                          \
"SYNOPSIS"                                                                          "\n"\
"    thread thread_create(execution, ...);"                                         "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `thread_create()` function starts a new native thread in the calling"      "\n"\
"    process, and create a new Lua VM to execute task."                             "\n"\
                                                                                    "\n"\
"    The new thread starts execution by invoking `execution`; Variable"             "\n"\
"    arguments will be passed to `execution` as **copy**. After `execution`"        "\n"\
"    finished, `callback` can be called in a new Lua thread (AKA. coroutine)."      "\n"\
                                                                                    "\n"\
"    The `execution` parameter can have any number of parameter as they will be"    "\n"\
"    copy and passed as it is. The `execution` parameter can have any number of"    "\n"\
"    return result."                                                                "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    Upon successful return, this function should return 0 indicate no"             "\n"\
"    error happen."                                                                 "\n"\
                                                                                    "\n"\
"    If there are any error, a error code will be returned. See `strerror()`"       "\n"\
"    for get string describing error number."                                       "\n"\
                                                                                    "\n"\
"NOTES"                                                                             "\n"\
"    The Lua VM does not fully support multi-thread, so it is not safe to"          "\n"\
"    directly access host VM in native thread, this is the reason why parameter"    "\n"\
"    need to copy and pass."                                                        "\n"\
                                                                                    "\n"\
"    Due to the copy progress and system delay, it is not recommend to execute"     "\n"\
"    light task (take only a few milliseconds or so) in native thread."             "\n"\
)

#define INFRA_LUA_API_THREAD_JOIN(XX)                                                   \
XX(                                                                                     \
"thread_join", infra_thread_join, NULL,                                                 \
"Waits for the thread to terminate.",                                                   \
"SYNOPSIS"                                                                          "\n"\
"    void thread_join(thread);"                                                     "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `thread_join()` function  waits for the thread specified to terminate."    "\n"\
"    If that thread has already terminated, then `thread_join()` returns"           "\n"\
"    immediately."                                                                  "\n"\
)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @see INFRA_LUA_API_THREAD_CREATE
 */
API_LOCAL int infra_thread_create(lua_State* L);

/**
 * @see INFRA_LUA_API_THREAD_JOIN
 */
API_LOCAL int infra_thread_join(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
