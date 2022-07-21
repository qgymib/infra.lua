#include "copy.h"
#include "lua_errno.h"
#include <assert.h>

static int _infra_copy_kv(lua_State* dst, lua_State* src)
{
    int errcode;

    /* Copy key */
    if ((errcode = infra_copy(dst, src, -2)) != 0)
    {
        return errcode;
    }

    /* Copy value */
    if ((errcode = infra_copy(dst, src, -1)) != 0)
    {
        lua_pop(dst, 1);    /* Remove key */
        return errcode;
    }

    /* t[key] = value */
    assert(lua_istable(dst, -3));
    lua_rawset(dst, -3);

    return 0;
}

static int _infra_copy_table(lua_State* dst, lua_State* src, int idx)
{
    int errcode = 0;
    lua_newtable(dst);

    lua_pushnil(src);
    while (lua_next(src, idx) && errcode == 0)
    {
        errcode = _infra_copy_kv(dst, src);

        /* Pop value */
        lua_pop(src, 1);
    }

    /* Pop key */
    lua_pop(src, 1);

    /* If error occur, remove table on dst stack */
    if (errcode != 0)
    {
        lua_pop(dst, 1);
    }

    return errcode;
}

static int _func_dumper(lua_State *L, const void *p, size_t sz, void *ud)
{
    (void)L;
    luaL_Buffer* b = ud;
    luaL_addlstring(b, p, sz);
    return 0;
}

/**
 * @brief Dump function at \p idx and push result on top of \L.
 * @param L     The Lua VM.
 * @param idx   The function index.
 * @return      Infra error code.
 */
static int _infra_dump_function(lua_State* L, int idx)
{
    luaL_Buffer B;
    luaL_buffinit(L, &B);

    /* `lua_dump()` require the function at the top of stack. */
    const int need_top = lua_gettop(L) != idx;
    if (need_top)
    {
        lua_pushvalue(L, idx);
    }

    /* Dump function into `B` */
    int errcode = lua_dump(L, _func_dumper, &B, 0);

    /* Pop function if necessary */
    if (need_top)
    {
        lua_pop(L, 1);
    }

    /* Push value to stack */
    if (errcode == 0)
    {
        luaL_pushresult(&B);
    }

    return errcode;
}

static int _infra_transfer_function(lua_State* dst, lua_State* src)
{
    size_t sz;
    const char* s = lua_tolstring(src, -1, &sz);

    int errcode = luaL_loadbuffer(dst, s, sz, NULL);
    if (errcode != LUA_OK)
    {
        /* no return */
        return luaL_error(src, "transfer function failed: %s", lua_tostring(dst, -1));
    }

    /* Remove dumped function */
    lua_pop(src, 1);

    return errcode;
}

static int _infra_copy_function(lua_State* dst, lua_State* src, int idx)
{
    int errcode;

    /* Dump and push function onto top of src */
    if ((errcode = _infra_dump_function(src, idx)) != 0)
    {
        return errcode;
    }

    /* Pop and transfer function to target Lua VM */
    errcode = _infra_transfer_function(dst, src);

    return errcode;
}

int infra_copy(lua_State* dst, lua_State* src, int idx)
{
    int value_type = lua_type(src, idx);
    if (value_type == LUA_TNONE)
    {
        return INFRA_ENOENT;
    }

    int errcode;
    switch (value_type)
    {
    case LUA_TNIL:
        lua_pushnil(dst);
        break;

    case LUA_TNUMBER:
        if (lua_isinteger(src, idx))
        {
            lua_pushinteger(dst, lua_tointeger(src, idx));
        }
        else
        {
            lua_pushnumber(dst, lua_tonumber(src, idx));
        }
        break;

    case LUA_TBOOLEAN:
        lua_pushboolean(dst, lua_toboolean(src, idx));
        break;

    case LUA_TSTRING:
        {
            size_t len;
            const char* s = lua_tolstring(src, idx, &len);
            lua_pushlstring(dst, s, len);
        }
        break;

    case LUA_TTABLE:
        if ((errcode = _infra_copy_table(dst, src, idx)) != 0)
        {
            return errcode;
        }
        break;

    case LUA_TFUNCTION:
        if ((errcode = _infra_copy_function(dst, src, idx)) != 0)
        {
            return errcode;
        }
        break;

    case LUA_TLIGHTUSERDATA:
        lua_pushlightuserdata(dst, lua_touserdata(src, idx));
        break;

    case LUA_TUSERDATA:
    case LUA_TTHREAD:
    default:
        return INFRA_ENOTSUP;
    }

    return INFRA_SUCCESS;
}
