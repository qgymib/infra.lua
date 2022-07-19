#ifndef __INFRA_API_PROMISE_H__
#define __INFRA_API_PROMISE_H__

#include "lua_api.h"

#define INFRA_LUA_API_NEW_PROMISE(XX)                                                   \
XX(                                                                                     \
"new_promise", infra_new_promise, infra_init_promise,                                   \
"Make a new promise",                                                                   \
"SYNOPSIS"                                                                          "\n"\
"    promise new_promise(callback);"                                                "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `new_promise()` function make a new promise object and return."            "\n"\
                                                                                    "\n"\
"    The `callback` must have following prototype:"                                 "\n"\
"    ```lua"                                                                        "\n"\
"    function callback(resolve, reject) end"                                        "\n"\
"    ```"                                                                           "\n"\
                                                                                    "\n"\
"    Both `resolve` and `reject` have prototype of"                                 "\n"\
"    ```lua"                                                                        "\n"\
"    function resolve_or_reject(data) end"                                          "\n"\
"    ```"                                                                           "\n"\
)

#if !HAVE_NEW_PROMISE
#undef INFRA_LUA_API_NEW_PROMISE
#define INFRA_LUA_API_NEW_PROMISE(XX)
#endif

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL int infra_new_promise(lua_State* L);

API_LOCAL void infra_init_promise(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
