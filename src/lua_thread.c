#include "lua_thread.h"
#include "lua_errno.h"
#include "utils/copy.h"
#include <uv.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#define INFRA_THREAD    "infra_thread_create"

typedef struct execution_context
{
    lua_State*          shadow_vm;
    lua_State*          co_thread;
    int                 nargs;

    uv_thread_t         sys_thread;
    int                 sys_thread_joined;

    uv_mutex_t          lock;
    unsigned            refcnt;
} execution_context_t;

static void _infra_thread_decref(execution_context_t* context)
{
    int refcnt;
    uv_mutex_lock(&context->lock);
    {
        context->refcnt--;
        refcnt = context->refcnt;
    }
    uv_mutex_unlock(&context->lock);

    if (refcnt != 0)
    {
        return;
    }

    uv_mutex_destroy(&context->lock);
}

static void _thread_body(void* arg)
{
    int i;
    execution_context_t* context = arg;

    /* Setup basic library */
    luaL_openlibs(context->shadow_vm);
    luaL_requiref(context->shadow_vm, "infra", luaopen_infra, 1);
    lua_pop(context->shadow_vm, 1);

    int ret = lua_pcall(context->shadow_vm, context->nargs, 0, LUA_MULTRET);

    if (ret != LUA_OK)
    {
        fprintf(stderr, "%s\n", lua_tostring(context->shadow_vm, -1));
    }

    /*
     * Push call result
     */
    lua_pushinteger(context->co_thread, ret);

    /*
     * Because we use a clean Lua VM, and `lua_pcall()` pop all arguments and
     * the function, now only return values left on the stack.
     */
    int n_element = lua_gettop(context->shadow_vm);

    /*
     * Copy all return value to coroutine
     */
    for (i = 1; i <= n_element; i++)
    {
        infra_copy(context->co_thread, context->shadow_vm, i);
    }

    _infra_thread_decref(context);
}

static int _on_thread_finish(lua_State* L)
{
    execution_context_t* context = lua_touserdata(L, lua_upvalueindex(1));

    /* Cleanup thread at the end */
    uv_thread_join(&context->sys_thread);

    return lua_gettop(L);
}

static int _thread_finalizer(lua_State* L)
{
    execution_context_t* context = lua_touserdata(L, 1);
    _infra_thread_decref(context);
    return 0;
}

static void _thread_set_metatable(lua_State* L)
{
    static const luaL_Reg s_thread_meta[] = {
        { "__gc", _thread_finalizer },
        { NULL, NULL },
    };

    if (luaL_newmetatable(L, INFRA_THREAD) != 0)
    {
        luaL_setfuncs(L, s_thread_meta, 0);
    }

    lua_setmetatable(L, -2);
}

static void _init_thread(lua_State* L, execution_context_t* context)
{
    context->refcnt = 2;
    context->sys_thread_joined = 0;
    uv_mutex_init(&context->lock);
    context->shadow_vm = luaL_newstate();
    context->co_thread = lua_newthread(L);
    lua_setuservalue(L, -2);
    _thread_set_metatable(L);
}

int infra_thread_create(lua_State* L)
{
    int errcode = INFRA_SUCCESS;

    execution_context_t* context = lua_newuserdata(L, sizeof(execution_context_t));
    _init_thread(L, context);

    lua_pushvalue(context->co_thread, -1);
    lua_pushcclosure(context->co_thread, _on_thread_finish, 1);

    context->nargs = 0;
    int top_id = lua_gettop(L);

    int i;
    for (i = 1; i <= top_id - 1; i++)
    {
        if ((errcode = infra_copy(context->shadow_vm, L, i)) != 0)
        {
            return luaL_error(L, "can not copy parameter at index %d: %s(%d)",
                i, infra_strerror_c(errcode), errcode);
        }
        context->nargs++;
    }

	if (uv_thread_create(&context->sys_thread, _thread_body, context) != 0)
    {
        errcode = infra_translate_sys_error(errno);
        return luaL_error(L, "create thread failed: %s(%d)",
            infra_strerror_c(errcode), errcode);
    }

    return 1;
}

int infra_thread_join(lua_State* L)
{
    execution_context_t* context = lua_touserdata(L, 1);
    assert(context != NULL);

    if (!context->sys_thread_joined)
    {
        context->sys_thread_joined = 1;
        uv_thread_join(&context->sys_thread);
    }

    return 0;
}
