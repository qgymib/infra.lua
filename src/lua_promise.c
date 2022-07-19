#include "lua_promise.h"
#include "lua_errno.h"
#include <assert.h>

#define INFRA_PROMISE   "infra_promise"

typedef enum infra_promise_uvidx
{
    INFRA_PROMISE_COROUTINE = 1,
    INFRA_PROMISE_CHAIN,
} infra_promise_uvidx_t;

typedef struct infra_promise
{
    lua_State*  co_thread;
} infra_promise_t;

static int _infra_promise_finalizer(lua_State* L)
{
    infra_promise_t* promise = lua_touserdata(L, 1);

    return 0;
}

static int _infra_promise_next(lua_State* L)
{

}

static int _infra_promise_catch(lua_State* L)
{

}

/**
 * @brief Check whether argument at \p index is a callable object.
 * @warning Stack must maintain balance.
 * @param[in] L         Lua VM.
 * @param[in] index     Argument index.
 * @return              Bool.
 */
static int _infra_promise_is_function(lua_State* L, int index)
{
    if (lua_isfunction(L, index))
    {
        return 1;
    }

    /* SP += 1 */
    if (lua_getmetatable(L, index))
    {
        return 0;
    }

    /* SP += 1 */
    int type = lua_getfield(L, -1, "__call");

    /* SP -= 2 */
    lua_pop(L, 2);

    return type == LUA_TFUNCTION;
}

static int _infra_promise_resolve(lua_State* L)
{

}

static int _infra_promise_reject(lua_State* L)
{

}

static int _infra_promise_setup(infra_promise_t* promise, lua_State* L)
{
    if (!_infra_promise_is_function(L, 1))
    {
        return INFRA_EINVAL;
    }

    /*
     * L:
     * ```
     *    TOP -> | USERDATA | (2)
     * BOTTOM -> | CALLBACK | (1)
     * ```
     */

    /*
     * We must swap two stack elements.
     * Move UserData to bottom of stack so that we can use `lua_setiuservalue()`
     * and `lua_xmove()`.
     */
    lua_insert(L, 1);

    /*
     * L:
     * ```
     *    TOP -> | CALLBACK | (2)
     * BOTTOM -> | USERDATA | (1)
     * ```
     */

    promise->co_thread = lua_newthread(L);

    /*
     * L:
     * ```
     *    TOP -> | THREAD   | (3)
     *           | CALLBACK | (2)
     * BOTTOM -> | USERDATA | (1)
     * ```
     */

    lua_setiuservalue(L, 1, INFRA_PROMISE_COROUTINE);

    /*
     * L:
     * ```
     *    TOP -> | CALLBACK | (2)
     * BOTTOM -> | USERDATA | (1)
     * ```
     */

    lua_xmove(L, promise->co_thread, 1);

    lua_pushcfunction(promise->co_thread, _infra_promise_resolve);
    lua_pushcfunction(promise->co_thread, _infra_promise_reject);

    /*
     * co_thread:
     * ```
     *    TOP -> | _infra_promise_reject  | (3)
     *           | _infra_promise_resolve | (2)
     * BOTTOM -> | CALLBACK               | (1)
     * ```
     */

    return 0;
}

static const luaL_Reg s_promise_meta[] = {
    { "__gc",   _infra_promise_finalizer },
    { NULL, NULL }
};

static const luaL_Reg s_promise_method[] = {
    { "next",   _infra_promise_next },
    { "catch",  _infra_promise_catch },
    { NULL, NULL }
};

void infra_init_promise(lua_State* L)
{
    int lib_id, meta_id;

    /* newclass = {} */
    lua_createtable(L, 0, 0);
    lib_id = lua_gettop(L);

    /* metatable = {} */
    luaL_newmetatable(L, INFRA_PROMISE);
    meta_id = lua_gettop(L);
    luaL_setfuncs(L, s_promise_meta, 0);

    /* metatable.__index = _methods */
    luaL_newlib(L, s_promise_method);
    lua_setfield(L, meta_id, "__index");

    /* metatable.__metatable = _meta */
    luaL_newlib(L, s_promise_meta);
    lua_setfield(L, meta_id, "__metatable");

    /* class.__metatable = metatable */
    lua_setmetatable(L, lib_id);

    /* _G["Foo"] = newclass */
    lua_setglobal(L, INFRA_PROMISE);
}

int infra_new_promise(lua_State* L)
{
    int errcode;

    infra_promise_t* promise = lua_newuserdata(L, sizeof(infra_promise_t));
    if ((errcode = _infra_promise_setup(promise, L)) != 0)
    {
        lua_pop(L, 1);
        lua_pushinteger(L, errcode);
        return 1;
    }
    luaL_setmetatable(L, INFRA_PROMISE);

    lua_resume(promise->co_thread, L, 2, 0);
    return 1;
}
