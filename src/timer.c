/* ----------------------------------------------------------------------------
  Copyright (c) 2018, Microsoft Research, Daan Leijen
  This is free software; you can redistribute it and/or modify it under the
  terms of the Apache License, Version 2.0. A copy of the License can be
  found in the file "license.txt" at the root of this distribution.
-----------------------------------------------------------------------------*/
#include "nodec.h"
#include "nodec-primitive.h"
#include "nodec-internal.h"
#include <assert.h>
#include <time.h>

static uv_handle_t* handle_of_timer(uv_timer_t* timer) {
  return (uv_handle_t*)timer;
}

uv_timer_t* nodec_timer_alloc() {
  uv_timer_t* timer = nodec_zero_alloc(uv_timer_t);
  nodec_check(uv_timer_init(async_loop(), timer));
  return timer;
}

static void _timer_close_cb(uv_handle_t* timer) {
  nodec_free(timer);
}

void nodec_timer_close(uv_timer_t* timer) {
  if (timer!=NULL) {
    uv_close(handle_of_timer(timer), &_timer_close_cb);
  }
}

void nodec_timer_free(uv_timer_t* timer, bool owner_release) {
  nodec_timer_close(timer);
  if (owner_release) nodec_owner_release(timer);  
}

void nodec_timer_freev(lh_value timerv) {
  nodec_timer_free((uv_timer_t*)lh_ptr_value(timerv),true);
}

static void _async_timer_resume(uv_timer_t* timer) {
  uv_req_t* req = (uv_req_t*)timer->data;
  async_req_resume(req, 0);
}

void async_wait(uint64_t timeout) {
  uv_timer_t* timer = nodec_timer_alloc();
  {defer(nodec_timer_freev, lh_value_ptr(timer)) {
    {using_req(uv_req_t, req) {  // use a dummy request so we can await the timer handle
      timer->data = req;
      nodec_check(uv_timer_start(timer, &_async_timer_resume, timeout, 0));
      async_await_owned(req, timer);
    }}
  }}
}

void async_yield() {
  async_wait(0);
}


/* ----------------------------------------------------------------------------
  Internal timeout routine to delay certain function calls.
  Used for cancelation resumptions
-----------------------------------------------------------------------------*/

typedef struct _timeout_args {
  uv_timeoutfun* cb;
  void*          arg;
} timeout_args;

static void _timeout_cb(uv_timer_t* timer) {
  if (timer==NULL) return;
  timeout_args args = *((timeout_args*)timer->data);
  nodec_free(timer->data);
  nodec_timer_free(timer, false);
  args.cb(args.arg);
}

uv_errno_t _uv_set_timeout(uv_loop_t* loop, uv_timeoutfun* cb, void* arg, uint64_t timeout) {
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



/* ----------------------------------------------------------------------------
  Get current Date in fixed size RFC 1123 format; 
  updated at most once a second for efficiency
  Sun, 06 Nov 1994 08:49:37 GMT
-----------------------------------------------------------------------------*/


#define INET_DATE_LEN 29

const char* nodec_inet_date( const time_t * const now )
{
  static char inet_date[INET_DATE_LEN + 1] = "Thu, 01 Jan 1972 00:00:00 GMT";
  static time_t inet_time = 0;

  static const char* days[7] =
    { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static const char* months[12] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

  if (*now == inet_time) return inet_date;
  struct tm tm;
  gmtime_s(&tm, now);
  strftime(inet_date, INET_DATE_LEN + 1, "---, %d --- %Y %H:%M:%S GMT", &tm);
  if (tm.tm_wday >= 0 && tm.tm_wday < 7) memcpy(inet_date, days[tm.tm_wday], 3);
  if (tm.tm_mon >= 0 && tm.tm_mon < 12) memcpy(inet_date + 8, months[tm.tm_mon], 3);
  return inet_date;
}

const char* nodec_inet_date_now() {
  time_t now;
  time(&now);
  return nodec_inet_date(&now);
}