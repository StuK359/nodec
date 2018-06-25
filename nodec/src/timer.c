/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the Apache License, Version 2.0. A copy of the License can be
found in the file "license.txt" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "nodec.h"
#include "nodec-internal.h"
#include <uv.h>
#include <assert.h>

static uv_handle_t* handle_of_timer(uv_timer_t* timer) {
  return (uv_handle_t*)timer;
}

uv_timer_t* nodec_timer_alloc() {
  uv_timer_t* timer = nodec_zero_alloc(uv_timer_t);
  check_uverr(uv_timer_init(async_loop(), timer));
  return timer;
}

static void _timer_close_cb(uv_handle_t* timer) {
  nodec_free(timer);
}

void nodec_timer_free(uv_timer_t* timer) {
  if (timer!=NULL) {
    uv_timer_stop(timer);
    uv_close(handle_of_timer(timer), &_timer_close_cb);
  }
}

void nodec_timer_freev(lh_value timerv) {
  nodec_timer_free((uv_timer_t*)lh_ptr_value(timerv));
}




typedef struct _timeout_args {
  uv_timeoutfun* cb;
  void*          arg;
} timeout_args;

static void _timeout_cb(uv_timer_t* timer) {
  if (timer==NULL) return;
  timeout_args args = *((timeout_args*)timer->data);
  nodec_free(timer->data);
  nodec_timer_free(timer);
  args.cb(args.arg);
}

uverr _uv_set_timeout(uv_loop_t* loop, uv_timeoutfun* cb, void* arg, uint64_t timeout) {
  uv_timer_t* timer = nodecx_alloc(uv_timer_t);
  if (timer == NULL) return UV_ENOMEM;
  timeout_args* args = nodecx_alloc(timeout_args);
  if (args==NULL) { nodec_free(timer); return UV_ENOMEM; }
  nodec_zero(uv_timer_t, timer);
  uv_timer_init(loop, timer);
  args->cb = cb;
  args->arg = arg;
  timer->data = args;
  return uv_timer_start(timer, &_timeout_cb, timeout, 0);
}


static void _async_timer_resume(uv_timer_t* timer) {
  uv_req_t* req = (uv_req_t*)timer->data;
  async_req_resume(req, 0);
}

void async_delay(uint64_t timeout) {
  uv_timer_t* timer = nodec_timer_alloc();
  {defer(nodec_timer_freev,lh_value_ptr(timer)){
    {with_free_req(uv_req_t, req) {  // use a dummy request so we can await the timer handle
                                     // and always free since we always close the timer too 
      timer->data = req;
      check_uverr(uv_timer_start(timer, &_async_timer_resume, timeout, 0));
      async_await(req);
    }}
  }}
}

void async_yield() {
  async_delay(0);
}
