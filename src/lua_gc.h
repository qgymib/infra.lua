#ifndef __INFRA_API_GC_H__
#define __INFRA_API_GC_H__

#include "lua_api.h"

#define INFRA_LUA_API_GC(XX)                                                            \
XX(                                                                                     \
"gc", infra_gc, NULL,                                                                   \
"Perform garbage collector right now.",                                                 \
"SYNOPSIS"                                                                          "\n"\
"    void gc();"                                                                    "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `gc()` perform garbage collector in calling thread."                       "\n"\
                                                                                    "\n"\
"    Normally you don't need to do it manually, because:"                           "\n"\
"    1. infra already do it for you in background."                                 "\n"\
"    2. infra only generate garbage when some functions was used inappropriate,"    "\n"\
"      like lost reference to active system object that can not be released"        "\n"\
"      immediately."                                                                "\n"\
)

#ifdef __cplusplus
extern "C" {
#endif

API_LOCAL int infra_gc(lua_State* L);

#ifdef __cplusplus
}
#endif
#endif
