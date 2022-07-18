#include "lua_scheduler.h"
#include "lua_errno.h"
#include "utils/list.h"
#include "utils/map.h"
#include <stdlib.h>
#include <assert.h>

#define INFRA_SCHEDULER "infra_scheduler"

typedef enum schedule_type
{
    SCHEDULE_BUSY,
    SCHEDULE_WAIT,
} schedule_type_t;

typedef struct coroutine_record
{
    ev_map_node_t           co_node;    /**< Node for record */
    ev_list_node_t          sc_node;    /**< Node for schedule */

    struct
    {
        lua_State*          co_thread;
        int                 co_ref_key;
        schedule_type_t     sc_type;
    } data;
} coroutine_record_t;

typedef struct infra_scheduler
{
    ev_map_t                co_map;

    ev_list_t               busy_queue;
    ev_list_t               wait_queue;
    ev_list_t               dead_queue;

    int                     flag_looping;   /**< Looping flag */
    size_t                  cnt_dead;       /**< Counter for dead coroutine */
} infra_scheduler_t;

static void _scheduler_cleanup_dead_thread(lua_State* L,
                                           infra_scheduler_t* scheduler)
{
    scheduler->cnt_dead += ev_list_size(&scheduler->dead_queue);

    ev_list_node_t* it;
    while ((it = ev_list_pop_front(&scheduler->dead_queue)) != NULL)
    {
        coroutine_record_t* rec = container_of(it, coroutine_record_t, sc_node);
        ev_map_erase(&scheduler->co_map, &rec->co_node);

        if (rec->data.co_ref_key != LUA_NOREF)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, rec->data.co_ref_key);
            rec->data.co_ref_key = LUA_NOREF;
        }
        rec->data.co_thread = NULL;

        free(rec);
    }
}

static int _scheduler_finalizer(lua_State* L)
{
    infra_scheduler_t* scheduler = lua_touserdata(L, 1);
    assert(scheduler != NULL);

    ev_list_migrate(&scheduler->dead_queue, &scheduler->busy_queue);
    ev_list_migrate(&scheduler->dead_queue, &scheduler->wait_queue);

    _scheduler_cleanup_dead_thread(L, scheduler);

    return 0;
}

static const luaL_Reg s_scheduler_meta[] = {
    { "__gc", _scheduler_finalizer },
    { NULL, NULL }
};

static int _scheduler_on_cmp(const ev_map_node_t* key1,
                             const ev_map_node_t* key2, void* arg)
{
    (void)arg;
    coroutine_record_t* rec_1 = container_of(key1, coroutine_record_t, co_node);
    coroutine_record_t* rec_2 = container_of(key2, coroutine_record_t, co_node);
    if (rec_1->data.co_thread == rec_2->data.co_thread)
    {
        return 0;
    }
    return rec_1->data.co_thread < rec_2->data.co_thread ? -1 : 1;
}

static void _scheduler_set_metatable(lua_State* L)
{
    if (luaL_newmetatable(L, INFRA_SCHEDULER) != 0)
    {
        luaL_setfuncs(L, s_scheduler_meta, 0);
    }

    lua_setmetatable(L, -2);
}

static int _scheduler_set_as_ready(infra_scheduler_t* scheduler, lua_State* thread)
{
    coroutine_record_t tmp_key;
    tmp_key.data.co_thread = thread;

    ev_map_node_t* it = ev_map_find(&scheduler->co_map, &tmp_key.co_node);
    if (it == NULL)
    {
        return INFRA_ENOENT;
    }

    coroutine_record_t* rec = container_of(it, coroutine_record_t, co_node);
    if (rec->data.sc_type == SCHEDULE_WAIT)
    {
        ev_list_erase(&scheduler->wait_queue, &rec->sc_node);
        ev_list_push_back(&scheduler->busy_queue, &rec->sc_node);
    }

    return INFRA_SUCCESS;
}

static int _scheduler_set_as_wait(infra_scheduler_t* scheduler, lua_State* thread)
{
    coroutine_record_t tmp_key;
    tmp_key.data.co_thread = thread;

    ev_map_node_t* it = ev_map_find(&scheduler->co_map, &tmp_key.co_node);
    if (it == NULL)
    {
        return INFRA_ENOENT;
    }

    coroutine_record_t* rec = container_of(it, coroutine_record_t, co_node);
    if (rec->data.sc_type == SCHEDULE_BUSY)
    {
        ev_list_erase(&scheduler->busy_queue, &rec->sc_node);
        ev_list_push_back(&scheduler->wait_queue, &rec->sc_node);
    }

    return INFRA_SUCCESS;
}

static void _scheduler_loop(lua_State* L, infra_scheduler_t* scheduler)
{
    ev_list_node_t* it = ev_list_begin(&scheduler->busy_queue);
    while (it != NULL)
    {
        coroutine_record_t* rec = container_of(it, coroutine_record_t, sc_node);
        it = ev_list_next(it);

        int nresults;
        int errcode = lua_resume(rec->data.co_thread, L, 0, &nresults);

        /* coroutine yields */
        if (errcode == LUA_YIELD)
        {
            continue;
        }

        /* finishes execution with or without errors */
        ev_list_erase(&scheduler->busy_queue, &rec->sc_node);
        ev_list_push_back(&scheduler->dead_queue, &rec->sc_node);
    }

    _scheduler_cleanup_dead_thread(L, scheduler);
}

static infra_scheduler_t* _scheduler_get_instance(lua_State* L)
{
    infra_scheduler_t* scheduler;
    int obj_type = lua_getglobal(L, INFRA_SCHEDULER);

    /* Check global scheduler */
    if (obj_type == LUA_TUSERDATA)
    {
        scheduler = lua_touserdata(L, -1);
        lua_pop(L, 1);
        return scheduler;
    }

    scheduler = lua_newuserdata(L, sizeof(infra_scheduler_t));
    assert(scheduler != NULL);

    ev_map_init(&scheduler->co_map, _scheduler_on_cmp, NULL);
    ev_list_init(&scheduler->busy_queue);
    ev_list_init(&scheduler->wait_queue);
    ev_list_init(&scheduler->dead_queue);

    scheduler->flag_looping = 0;
    scheduler->cnt_dead = 0;
    _scheduler_set_metatable(L);

    lua_setglobal(L, INFRA_SCHEDULER);

    return scheduler;
}

int infra_scheduler_run(lua_State* L)
{
    infra_scheduler_t* scheduler = _scheduler_get_instance(L);
    assert(scheduler != NULL);

    scheduler->flag_looping = 1;
    while (scheduler->flag_looping && ev_list_size(&scheduler->busy_queue) > 0)
    {
        _scheduler_loop(L, scheduler);
    }

    lua_pushinteger(L, 0);
    return 1;
}

int infra_scheduler_spawn(lua_State* L)
{
    int ret = INFRA_SUCCESS;

    infra_scheduler_t* scheduler = _scheduler_get_instance(L);
    assert(scheduler != NULL);

    lua_State* thread = lua_tothread(L, 1);
    assert(thread != NULL);

    coroutine_record_t* rec = malloc(sizeof(coroutine_record_t));
    assert(rec != NULL);

    rec->data.co_thread = thread;
    rec->data.co_ref_key = LUA_NOREF;   // update later
    rec->data.sc_type = SCHEDULE_BUSY;

    if (ev_map_insert(&scheduler->co_map, &rec->co_node) != 0)
    {/* Record already exist */
        ret = INFRA_EEXIST;
        free(rec);
        goto fin;
    }

    rec->data.co_ref_key = luaL_ref(L, LUA_REGISTRYINDEX);
    ev_list_push_back(&scheduler->busy_queue, &rec->sc_node);

fin:
    lua_pushinteger(L, ret);
    return 1;
}

int infra_scheduler_ready(lua_State* L)
{
    infra_scheduler_t* scheduler = _scheduler_get_instance(L);
    assert(scheduler != NULL);

    lua_State* thread = lua_tothread(L, 1);
    assert(thread != NULL);

    int ret = _scheduler_set_as_ready(scheduler, thread);
    lua_pushinteger(L, ret);

    return 1;
}

int infra_scheduler_wait(lua_State* L)
{
    infra_scheduler_t* scheduler = _scheduler_get_instance(L);
    assert(scheduler != NULL);

    lua_State* thread = lua_tothread(L, 1);
    assert(thread != NULL);

    int ret = _scheduler_set_as_wait(scheduler, thread);
    lua_pushinteger(L, ret);

    return 1;
}

int infra_scheduler_info(lua_State* L)
{
    infra_scheduler_t* scheduler = _scheduler_get_instance(L);
    assert(scheduler != NULL);

    lua_newtable(L);

    lua_pushstring(L, "busy");
    lua_pushinteger(L, ev_list_size(&scheduler->busy_queue));
    lua_settable(L, -3);

    lua_pushstring(L, "wait");
    lua_pushinteger(L, ev_list_size(&scheduler->wait_queue));
    lua_settable(L, -3);

    lua_pushstring(L, "dead");
    lua_pushinteger(L, scheduler->cnt_dead);
    lua_settable(L, -3);

    return 1;
}
