#include "lua_dump.h"

static void _dump_value_as_string_at_index(luaL_Buffer* buf, lua_State* L, int idx, int level);

static void _dump_add_prefix(luaL_Buffer* buf, int level)
{
    int i;
    for (i = 0; i < level; i++)
    {
        luaL_addstring(buf, "    ");
    }
}

static void _dump_table_as_string(luaL_Buffer* buf, lua_State* L, int idx, int level)
{
    luaL_addstring(buf, "{\n");

    int top = lua_gettop(L);

    lua_pushnil(L);
    while (lua_next(L, idx) != 0)
    {
        _dump_value_as_string_at_index(buf, L, top + 1, level + 1);
        luaL_addstring(buf, " = ");
        _dump_value_as_string_at_index(buf, L, top + 2, 0);
        luaL_addstring(buf, ",\n");

        lua_pop(L, 1);
    }

    _dump_add_prefix(buf, level);
    luaL_addstring(buf, "}");
}

static void _dump_value_as_string_at_index(luaL_Buffer* buf, lua_State* L, int idx, int level)
{
    char buffer[64];

    _dump_add_prefix(buf, level);

    int val_type = lua_type(L, idx);
    switch(val_type)
    {
    case LUA_TNIL:
        luaL_addstring(buf, "<NIL>");
        break;

    case LUA_TNUMBER:
        if (lua_isinteger(L, idx))
        {
            lua_Integer val = lua_tointeger(L, idx);
            snprintf(buffer, sizeof(buffer), LUA_INTEGER_FMT, val);
        }
        else
        {
            lua_Number val = lua_tonumber(L, idx);
            snprintf(buffer, sizeof(buffer), LUA_NUMBER_FMT, val);
        }
        luaL_addstring(buf, buffer);
        break;

    case LUA_TBOOLEAN:
        luaL_addstring(buf, lua_toboolean(L, idx) ? "<BOOLEAN>:TRUE" : "<BOOLEAN>:FALSE");
        break;

    case LUA_TSTRING:
        luaL_addstring(buf, lua_tostring(L, idx));
        break;

    case LUA_TTABLE:
        snprintf(buffer, sizeof(buffer), "<TABLE:%p>", lua_topointer(L, idx));
        luaL_addstring(buf, buffer);
        _dump_table_as_string(buf, L, idx, level);
        break;

    case LUA_TFUNCTION:
        if (lua_iscfunction(L, idx))
        {
            snprintf(buffer, sizeof(buffer), "<C_FUNCTION:%p>", lua_tocfunction(L, idx));
            luaL_addstring(buf, buffer);
        }
        else
        {
            lua_Debug ar;
            lua_pushvalue(L, idx);
            lua_getinfo(L, ">n", &ar);

            snprintf(buffer, sizeof(buffer), "<LUA_FUNCTION:%p>", lua_topointer(L, idx));
            luaL_addstring(buf, buffer);
            luaL_addstring(buf, ar.name);
        }
        break;

    case LUA_TUSERDATA:
        snprintf(buffer, sizeof(buffer), "<USERDATA:%p>", lua_touserdata(L, idx));
        luaL_addstring(buf, buffer);
        break;

    case LUA_TTHREAD:
        snprintf(buffer, sizeof(buffer), "<THREAD:%p>", lua_topointer(L, idx));
        luaL_addstring(buf, buffer);
        break;

    case LUA_TLIGHTUSERDATA:
        snprintf(buffer, sizeof(buffer), "<LIGHTUSERDATA:%p>", lua_touserdata(L, idx));
        luaL_addstring(buf, buffer);
        break;

    default:
        snprintf(buffer, sizeof(buffer), "<UNKNOWN:%p>", lua_topointer(L, idx));
        luaL_addstring(buf, buffer);
        break;
    }
}

int infra_dump_value_as_string(lua_State* L)
{
    luaL_Buffer buf;
    luaL_buffinit(L, &buf);

    _dump_value_as_string_at_index(&buf, L, 1, 0);
    luaL_pushresult(&buf);

    return 1;
}
