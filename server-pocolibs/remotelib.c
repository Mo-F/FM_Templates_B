<'
# Copyright (c) 2012-2014 LAAS/CNRS
# All rights reserved.
#
# Redistribution and use  in source  and binary  forms,  with or without
# modification, are permitted provided that the following conditions are
# met:
#
#   1. Redistributions of  source  code must retain the  above copyright
#      notice and this list of conditions.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice and  this list of  conditions in the  documentation and/or
#      other materials provided with the distribution.
#
#                                      Anthony Mallet on Sun Jul 29 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <limits.h>
#include <stdarg.h>

#include "<"$comp">_control_task.h"
#include "<"$comp">_remotelib.h"

<'if {[llength [$component remotes]]} {'>

/* --- genom_remote_vacall ------------------------------------------------- */

/* For each remote, the user-visible 'call()' function merely calls a generic
 * function defined with variadic arguments. This is so because a/ it provides
 * a clean function interface to the user and b/ the variadic version allows us
 * to implement a serialization function that serializes into the csLib buffer
 * with no explicit copy (and the inline does the rest, so that with -O there
 * should not be any superfluous copy). */

static __inline__ genom_event
genom_remote_vacall(struct genom_remote *r,
                    const char *remote,
                    int (*encode)(char *, int, char *, int),
                    int (*decode)(char *, int, char *, int),
                    genom_context self, ...)
{
  va_list ap;
  STATUS s;
  int aid, rid;

  if (!r->csid) return genom_remote_io(self);

  /* send */
  va_start(ap, self);
  s = csClientRqstSend(r->csid, r->meta.services._buffer[r->id].rqstid,
                       (char *)&ap, 0, encode, TRUE, 0, 0, &rid);
  va_end(ap);
  if (s != OK) {
    int e = errnoGet();
    genom_log_warn(1, "remote %s send", remote);
    if (H2_SYS_ERR_FLAG(e))
      return genom_syserr(&(genom_syserr_detail){ .code = e }, self);
    else {
      genom_mwerr_detail d;
      h2getErrMsg(e, d.what, sizeof(d.what));
      return genom_mwerr(&d, self);
    }
    assert(!"not reached");
  }

  /* receive */
  va_start(ap, self);
  s = csClientReplyRcv(
    r->csid, rid,
    BLOCK_ON_FINAL_REPLY,		/* NO_BLOCK, BLOCK_ON_INTERMED_REPLY */
    (char *)&aid, sizeof(aid), NULL,
    (char *)&ap, 0, decode);
  va_end(ap);

  if (s != FINAL_REPLY_OK) {
    genom_event ex;
    int e = errnoGet();
    if (e != ERROR) {
      genom_log_warn(1, "remote %s receive", remote);
      if (H2_SYS_ERR_FLAG(e))
        return genom_syserr(&(genom_syserr_detail){ .code = e }, self);
      else {
        genom_mwerr_detail d;
        h2getErrMsg(e, d.what, sizeof(d.what));
        return genom_mwerr(&d, self);
      }
      assert(!"not reached");
    }
    self->raised(&ex, self);
    return ex;
  }
  return genom_ok;
}


/* --- genom_remote_lookup ------------------------------------------------- */

static genom_event
genom_remote_lookup(struct genom_component_data *self,
                    struct genom_remote *r, const char *remote,
                    const char *digest, const char *srv, const char *svc)
{
  genom_event s;
  int i;

  /* get remote rqstid */
  s = genom_metadata_<"$comp">_fetch(self, srv, &r->meta);
  if (s) return s;

  r->id = -1;
  for(i = 0; i<r->meta.services._length; i++)
    if (!strcmp(r->meta.services._buffer[i].name, svc)) {
      r->id = i;
      break;
    }
  if (r->id < 0) {
    genom_log_warn(0, "service %s is not provided by %s", svc, srv);
    return genom_no_such_service(&self->context);
  }

  /* check interface compatibility */
  if (strcmp(digest, r->meta.services._buffer[r->id].digest)) {
    genom_log_warn(0, "service %s/%s is incompatible with remote %s",
                   srv, svc, remote);
    return genom_no_such_service(&self->context);
  }

  return genom_ok;
}
<'}'>

<'foreach r [$component remotes] {'>

/* <"[--- remote [$r name] ==============================================]"> */


/* <"[--- genom_${comp}_[$r name]_encode --------------------------------]"> */

static int
genom_<"$comp">_<"[$r name]">_encode(
  char *buffer, int size, char *dst, int maxsize)
{
<'  if {[llength [$r parameters in inout]]} {'>
  va_list *ap = (va_list *)buffer;
  const void *in;
  char *p = dst;
  size_t s;

  s = 0;
  va_arg(*ap, struct genom_context_iface *); /* skip context */

<'    foreach p [$r parameters] {'>
  /* <"[$p name]"> */
  in = va_arg(*ap, void *);
<'      if {[$p dir] == "out"} continue'>
  s += genom_serialen_<"[[$p type] mangle]">(in);
  if (s > maxsize) return ERROR;
  genom_serialize_<"[[$p type] mangle]">(&p, in);
<'    }'>

  return s;
<'  } else {'>
  return 0;
<'  }'>
}

/* <"[--- genom_${comp}_[$r name]_decode --------------------------------]"> */

static int
genom_<"$comp">_<"[$r name]">_decode(
  char *buffer, int size, char *dst, int maxsize)
{
  va_list *ap = (va_list *)dst;
  struct genom_context_iface *self;
  ssize_t len = size;
  char *p = buffer;
  int s;
  if (size == 0) return 0;

  self = va_arg(*ap, struct genom_context_iface *);

  if (*p == *"") {
    p++; len--;

<'    foreach p [$r parameters] {'>
    /* <"[$p name]"> */
<'      if {[$p dir] == "in"} {'>
    va_arg(*ap, void *);
<'      } else {'>
    s = genom_deserialize_<"[[$p type] mangle]">(&p, &len, va_arg(*ap, void *));
    if (s) {
      switch(s) {
        case ENOMSG: errnoSet(S_gcomLib_SMALL_DATA_STR);
        case ENOMEM: errnoSet(S_gcomLib_MALLOC_FAILED);
        default: errnoSet(S_gcomLib_NOT_A_LETTER);
      }
      return ERROR;
    }
<'      }'>
<'    }'>
<'    foreach e [$r throws] {'>
  } else if (!strcmp(p, <"[$e cname]">_id)) {
<'      if {[llength [$e members]]} {'>
    <"[$e declarator]"> exdetail;
    p += sizeof(<"[$e cname]">_id); len -= sizeof(<"[$e cname]">_id);
    genom_tinit_<"[$e mangle]">(&exdetail);
    s = genom_deserialize_<"[$e mangle]">(&p, &len, &exdetail);
    if (!s) <"[$e cname]">(&exdetail, self);
<'      } else {'>
    <"[$e cname]">(self);
    s = 0; len -= sizeof(<"[$e cname]">_id);
<'      }'>
    errnoSet(ERROR);
    size = ERROR;
<'    }'>
  } else if (!strcmp(p, genom_syserr_id)) {
    genom_syserr_detail exdetail;
    p += sizeof(genom_syserr_id); len -= sizeof(genom_syserr_id);
    memcpy(&exdetail, p, sizeof(exdetail));
    len -= sizeof(exdetail);
    genom_syserr(&exdetail, self);
    errnoSet(ERROR);
    size = ERROR;
    s = 0;
  } else if (!strcmp(p, genom_unkex_id)) {
    genom_unkex_detail exdetail;
    p += sizeof(genom_unkex_id); len -= sizeof(genom_unkex_id);
    memcpy(&exdetail, p, sizeof(exdetail));
    len -= sizeof(exdetail);
    genom_unkex(&exdetail, self);
    errnoSet(ERROR);
    size = ERROR;
    s = 0;
  } else {
    genom_unkex_detail d;
    strncpy(d.what, p, sizeof(d.what)); d.what[sizeof(d.what)-1] = *"";
    genom_unkex(&d, self);
    errnoSet(ERROR);
    return ERROR;
  }

  if (len != 0) {
    errnoSet(S_gcomLib_SMALL_LETTER);
    return ERROR;
  }
  return size;
}

/* <"[--- genom_${comp}_[$r name]_call ----------------------------------]"> */

<'
  set arg [list]
  foreach p [$r parameters] {
    switch -- [$p dir] {
      in		{ set a [[$p type] argument value [$p name]] }
      out - inout	{ set a [[$p type] argument reference [$p name]] }
      default		{ error "invalid parameter direction" }
    }
    lappend arg $a
  }
  lappend arg {genom_context self}
  set arg [join $arg {, }]
'>
static genom_event
genom_<"$comp">_<"[$r name]">_call(<"$arg">)
{
  /* sanity checks: we're going to interpret arguments as void * */
<'  set args [list]; foreach p [$r parameters] { '>
  switch(0){ case 0: case sizeof(<"[$p name]">) == sizeof(void *):; }
<'  lappend args [$p name] }'>
<'  if {![llength $args]} { lappend args NULL }'>
  return genom_remote_vacall(
    &self->data->self->remotes.<"[$r name]">.h,
    "<"[$r name]">",
    genom_<"$comp">_<"[$r name]">_encode,
    genom_<"$comp">_<"[$r name]">_decode,
    self, self, <"[join $args {, }]">);
}


/* <"[--- genom_${comp}_[$r name]_connect -------------------------------]"> */

genom_event
genom_<"$comp">_<"[$r name]">_connect(const char *name, genom_context self)
{
  struct genom_<"$comp">_<"[$r name]">_remote *r =
    &self->data->self->remotes.<"[$r name]">;
  char srv[PATH_MAX];
  genom_event e;
  char *svc;
  STATUS s;

  /* initialize csLib client */
  strncpy(srv, name, sizeof(srv)); srv[sizeof(srv)-1] = 0;
  svc = strchr(srv, '/');
  if (!svc) {
    genom_log_warn(0, "remote service %s should be component/service", name);
    return genom_no_such_service(self);
  }
  *(svc++) = 0;

  genom_tfini_<"$comp">_<"[$r name]">_remote(r);
  s = csClientInit(srv, genom_<"$comp">_<"[$r name]">_maxrqst_size(),
                   sizeof(int), genom_<"$comp">_<"[$r name]">_maxreply_size(),
                   &r->h.csid);
  if (s != OK) {
    genom_log_warn(1, "component %s could not be found", srv);
    r->h.csid = NULL;
    return genom_no_such_service(self);
  }

  /* get remote rqstid */
  e = genom_remote_lookup(self->data->self, &r->h, "<"[$r name]">",
                          "<"[$r digest]">", srv, svc);
  if (e) {
    genom_tfini_<"$comp">_<"[$r name]">_remote(r);
    return e;
  }

  r->data.call = genom_<"$comp">_<"[$r name]">_call;
  return genom_ok;
}

<'}'>

/* --- end of file --------------------------------------------------------- */
