#ifndef __INFRA_API_ERRNO_H__
#define __INFRA_API_ERRNO_H__

#include "lua_api.h"

#define INFRA_LUA_API_STRERROR(XX)                                                      \
XX(                                                                                     \
"strerror", infra_strerror, NULL,                                                       \
"return string describing error number",                                                \
"SYNOPSIS"                                                                          "\n"\
"    string strerror(errcode);"                                                     "\n"\
                                                                                    "\n"\
"    In infra library, all error code is a negative integer, which allow you to"    "\n"\
"    easily known whether an operation is a success or not."                        "\n"\
                                                                                    "\n"\
"DESCRIPTION"                                                                       "\n"\
"    The `strerror()` function returns a string that describes the error code"      "\n"\
"    passed in the argument `errcode`."                                             "\n"\
                                                                                    "\n"\
"RETURN VALUE"                                                                      "\n"\
"    On success, a string will be returned. On error, nil will be returned."        "\n"\
)

#if !HAVE_STRERROR
#undef INFRA_LUA_API_STRERROR
#define INFRA_LUA_API_STRERROR(XX)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum infra_errno
{
    INFRA_SUCCESS,
} infra_errno_t;

API_LOCAL int infra_strerror(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
