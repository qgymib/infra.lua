#ifndef __INFRA_API_SCHEDULER_H__
#define __INFRA_API_SCHEDULER_H__

#include "lua_api.h"

#define INFRA_LUA_API_SCHEDULER_RUN(XX)                                                 \
XX(                                                                                     \
"scheduler_run", infra_scheduler_run, NULL,                                             \
"Schedule all registered coroutines until all coroutine is dead.",                      \
"SYNOPSIS"                                                                          "\n"\
"    integer scheduler_run();"                                                      "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    Run a scheduler until all coroutine finished or was interrupted."              "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    On success, the `scheduler_run()` return 0. If the scheduler was"              "\n"\
"    interrupted, the number of registered thread is returned."                     "\n"\
)

#if !HAVE_SCHEDULER_RUN
#undef INFRA_LUA_API_SCHEDULER_RUN
#define INFRA_LUA_API_SCHEDULER_RUN(XX)
#endif

#define INFRA_LUA_API_SCHEDULER_SPAWN(XX)                                               \
XX(                                                                                     \
"scheduler_spawn", infra_scheduler_spawn, NULL,                                         \
"Accepts a thread and resumes it as soon as possible.",                                 \
"SYNOPSIS"                                                                          "\n"\
"    errno scheduler_spawn(coroutine, ...);"                                        "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `scheduler_spawn()` function take a thread and put it into end of"         "\n"\
"    scheduling queue."                                                             "\n"\
                                                                                    "\n"\
"    The coroutine will not be scheduled until somebody set it as ready with"       "\n"\
"    `scheduler_ready()`."                                                          "\n"\
                                                                                    "\n"\
"    You can not register the same coroutine twice."                                "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    On success, `scheduler_spawn()` return 0. On error, a negative error code"     "\n"\
"    is returned."                                                                  "\n"\
)

#if !HAVE_SCHEDULER_SPAWN
#undef INFRA_LUA_API_SCHEDULER_SPAWN
#define INFRA_LUA_API_SCHEDULER_SPAWN(XX)
#endif

#define INFRA_LUA_API_SCHEDULER_READY(XX)                                               \
XX(                                                                                     \
"scheduler_ready", infra_scheduler_ready, NULL,                                         \
"Set coroutine as ready to be scheduled",                                               \
"SYNOPSIS"                                                                          "\n"\
"    integer scheduler_ready(coroutine);"                                           "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `scheduler_ready()` function set a coroutine as READY, so scheduler is"    "\n"\
"    allowed to schedule it."                                                       "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    On success, `scheduler_ready()` return 0. If failed, an error code is"         "\n"\
"    returned."                                                                     "\n"\
)

#if !HAVE_SCHEDULER_READY
#undef INFRA_LUA_API_SCHEDULER_READY
#define INFRA_LUA_API_SCHEDULER_READY(XX)
#endif

#define INFRA_LUA_API_SCHEDULER_WAIT(XX)                                                \
XX(                                                                                     \
"scheduler_wait", infra_scheduler_wait, NULL,                                           \
"Set coroutine as wait infinite.",                                                      \
"SYNOPSIS"                                                                          "\n"\
"    integer scheduler_wait(coroutine);"                                            "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `scheduler_wait()` function set a coroutine as WAIT. A coroutine in"       "\n"\
"    WAIT status will not be scheduled unless use `scheduler_ready()` to set it"    "\n"\
"    as READY."                                                                     "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    On success, `scheduler_wait()` return 0. If failed, an error code is"          "\n"\
"    returned."                                                                     "\n"\
)

#if !HAVE_SCHEDULER_WAIT
#undef INFRA_LUA_API_SCHEDULER_WAIT
#define INFRA_LUA_API_SCHEDULER_WAIT(XX)
#endif

#define INFRA_LUA_API_SCHEDULER_INFO(XX)                                                \
XX(                                                                                     \
"scheduler_info", infra_scheduler_info, NULL,                                           \
"Get information about this scheduler.",                                                \
"SYNOPSIS"                                                                          "\n"\
"    table scheduler_info();"                                                       "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    The `scheduler_info()` function return a table for schedule information."      "\n"\
                                                                                    "\n"\
"    The table have following structure:"                                           "\n"\
"    ```lua"                                                                        "\n"\
"    {"                                                                             "\n"\
"        [busy] = integer,"                                                         "\n"\
"        [wait] = integer,"                                                         "\n"\
"        [dead] = integer,"                                                         "\n"\
"    }"                                                                             "\n"\
"    ```"                                                                           "\n"\
"    Each field have following means:"                                              "\n"\
"    + `busy`: The number of threads currently running."                            "\n"\
"    + `wait`: The number of threads wait to schedule (that called"                 "\n"\
"      `scheduler_wait()`)"                                                         "\n"\
"    + `dead`: The number of threads that already dead."                            "\n"\
)

#if !HAVE_SCHEDULER_INFO
#undef INFRA_LUA_API_SCHEDULER_INFO
#define INFRA_LUA_API_SCHEDULER_INFO(XX)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @see INFRA_LUA_API_SCHEDULER_RUN
 */
API_LOCAL int infra_scheduler_run(lua_State* L);

/**
 * @see INFRA_LUA_API_SCHEDULER_SPAWN
 */
API_LOCAL int infra_scheduler_spawn(lua_State* L);

/**
 * @see INFRA_LUA_API_SCHEDULER_READY
 */
API_LOCAL int infra_scheduler_ready(lua_State* L);

/**
 * @see INFRA_LUA_API_SCHEDULER_WAIT
 */
API_LOCAL int infra_scheduler_wait(lua_State* L);

/**
 * @see INFRA_LUA_API_SCHEDULER_INFO
 */
API_LOCAL int infra_scheduler_info(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
