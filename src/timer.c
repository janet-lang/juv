
static const JanetAbstractType timer_type = {
    "uv/timer",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static uv_timer_t* juv_gettimer(const Janet *argv, int32_t n) {
  uv_timer_t* handle = (uv_timer_t*) janet_getabstract(argv, n, &timer_type);
  return handle;
}

static int luv_new_timer(lua_State* L) {
  uv_timer_t* handle = (uv_timer_t*) janet_abstract(sizeof(*handle));
  int ret = uv_timer_init(uv_default_loop(), handle);
  if (ret < 0) {
      janet_panic("could not create timer");
  }
  handle->data = luv_setup_handle(L);
  return 1;
}

static void luv_timer_cb(uv_timer_t* handle) {
  lua_State* L = luv_state(handle->loop);
  luv_handle_t* data = (luv_handle_t*)handle->data;
  luv_call_callback(L, data, LUV_TIMEOUT, 0);
}

static int luv_timer_start(lua_State* L) {
  uv_timer_t* handle = luv_check_timer(L, 1);
  uint64_t timeout;
  uint64_t repeat;
  int ret;
  timeout = luaL_checkinteger(L, 2);
  repeat = luaL_checkinteger(L, 3);
  luv_check_callback(L, (luv_handle_t*)handle->data, LUV_TIMEOUT, 4);
  ret = uv_timer_start(handle, luv_timer_cb, timeout, repeat);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_timer_stop(lua_State* L) {
  uv_timer_t* handle = luv_check_timer(L, 1);
  int ret = uv_timer_stop(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_timer_again(lua_State* L) {
  uv_timer_t* handle = luv_check_timer(L, 1);
  int ret = uv_timer_again(handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_timer_set_repeat(lua_State* L) {
  uv_timer_t* handle = luv_check_timer(L, 1);
  uint64_t repeat = luaL_checkinteger(L, 2);
  uv_timer_set_repeat(handle, repeat);
  return 0;
}

static int luv_timer_get_repeat(lua_State* L) {
  uv_timer_t* handle = luv_check_timer(L, 1);
  uint64_t repeat = uv_timer_get_repeat(handle);
  lua_pushinteger(L, repeat);
  return 1;
}
