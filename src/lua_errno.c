#include "lua_errno.h"

#if HAVE_STRERROR

int infra_strerror(lua_State* L)
{
    const char* err_str = NULL;
    int errcode = (int)luaL_checkinteger(L, 1);
    switch (errcode)
    {
    case INFRA_SUCCESS:
        err_str = "Operation success";
        break;

    default:
        break;
    }

    if (err_str == NULL)
    {
        lua_pushnil(L);
    }
    else
    {
        lua_pushstring(L, err_str);
    }

    return 1;
}

#endif
