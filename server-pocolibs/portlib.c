<'
# Copyright (c) 2011-2016 LAAS/CNRS
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
#                                      Anthony Mallet on Fri Mar  9 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "h2devLib.h"

#include "<"$comp">_control_task.h"
#include "<"$comp">_portlib.h"
#include "serialize.h"

/* --- local data ---------------------------------------------------------- */

enum {
  <"$comp">_PORTLIB_MIN_INDEX = 0,
<'foreach p [$component ports] {'>
  <"$comp">_<"[$p name]">_INDEX,
<'}'>
  <"$comp">_PORTLIB_MAX_INDEX
};

/* --- state port ---------------------------------------------------------- */

int
genom_state_<"$comp">_init(struct genom_component_data *self)
{
  genom_state_component *data = &self->ports.genom_state.h.buffer;

  /* task list */
<'set tasks [$component tasks]'>
  if (genom_sequence_reserve(&data->task, <"[llength $tasks]">))
    return ENOMEM;
<'foreach t $tasks {'>
  {
    genom_state_task *task;
    task = &data->task._buffer[<"${comp}_[$t name]_TASKID">];

    snprintf(task->name, sizeof(task->name), "%s", "<"[$t name]">");
    task->activity._length = 0;
    if (genom_sequence_reserve(&task->activity, genom_max_activities()))
      return ENOMEM;
  }
<'}'>
  data->task._length = <"[llength $tasks]">;

  /* automatic versioning - nice idea borrowed from ROS */
<'
# one must filter out from the digest the 'metadata' port that is private
set digest [$component digest {{v} {
  if {[$v class] == "port" && [$v name] == "genom_metadata"} { return 0 }
  return 1
}}]
'>
  strncpy(data->digest, "<"$digest">", sizeof(data->digest));
  strncpy(data->date, "<"[clock format [clock sec]]">", sizeof(data->date));
  strncpy(data->version, "<"[$component version]">", sizeof(data->version));

  genom_log_debug("initialized genom_state port");
  return 0;
}

int
genom_state_<"$comp">_update(struct genom_component_data *self,
                    struct genom_activities *activities, int tnum)
{
  genom_state_component *data = &self->ports.genom_state.h.buffer;
  genom_state_task *t = &data->task._buffer[tnum];
  struct genom_activity *a;
  const char *s;
  size_t i, j;

  for(i = j = 0; i < genom_max_activities(); i++) {
    a = &activities->a[i].any;
    if (a->status == ACT_VOID) continue;

    t->activity._buffer[j].id = a->aid;
    switch(a->sid) {
      case -1: s = "<task>"; break;
<'foreach s [$component services] {'>
<'  if {[$s kind] ne "activity"} continue'>
      case <"$COMP">_<"[$s name]">_RQSTID: s = "<"[$s name]">"; break;
<'}'>
      default: assert(!"unknown service id"); break;
    }
    snprintf(t->activity._buffer[j].name,
             sizeof(t->activity._buffer[j].name), "%s", s);
    j++;
  }
  t->activity._length = j;
  return 0;
}

genom_event
genom_state_<"$comp">_refresh(struct genom_component_data *self)
{
<'if {[llength [$component tasks]]} {'>
  genom_state_component *data = &self->ports.genom_state.h.buffer;
  genom_state_task *task;
  genom_event s;
<'}'>


<'if {[llength [$component tasks]]} {'>
  s = self->ports.genom_state.handle.write(&self->control.context);
  if (s) genom_log_warn(0, "cannot update state port: %s", s);
  genom_log_debug("refreshed genom_state port");
  return s;
<'} else {'>
  return genom_ok;
<'}'>
}


/* --- metadata port ------------------------------------------------------- */

int
genom_metadata_<"$comp">_init(struct genom_component_data *self)
{
  pocolibs_metadata_component *meta = &self->ports.genom_metadata.h.buffer;
  size_t l, m;

  genom_sequence_reserve(&meta->services, <"[llength [$component services]]">);
  l = sizeof(meta->services._buffer->name);
  m = sizeof(meta->services._buffer->digest);
<'set i 0; foreach s [$component services] {'>
  meta->services._buffer[<"$i">].rqstid = <"$COMP">_<"[$s name]">_RQSTID;
  strncpy(meta->services._buffer[<"$i">].name, "<"[$s name]">", l);
  meta->services._buffer[<"$i">].name[l-1] = 0;
  strncpy(meta->services._buffer[<"$i">].digest, "<"[$s digest]">", m);
  meta->services._buffer[<"$i">].digest[m-1] = 0;
<'  incr i }'>
  meta->services._length = <"[llength [$component services]]">;

  genom_log_debug("initialized metadata port");
  return 0;
}

genom_event
genom_metadata_<"$comp">_fetch(struct genom_component_data *self,
                      const char *comp, pocolibs_metadata_component *meta)
{
  char name[H2_DEV_MAX_NAME];
  POSTER_ID id;
  char *data;
  STATUS s;
  ssize_t m;
  int n;

  n = snprintf(name, sizeof(name), "%s/genom_metadata", comp);
  if (n <= 0 || n >= sizeof(name)) {
    genom_log_warn(0, "port %s name too long", name);
    genom_log_warn(0, "port name length limited to %d characters",
                   H2_DEV_MAX_NAME);
    return genom_syserr(&(genom_syserr_detail){ .code = ENAMETOOLONG },
                        &self->control.context);
  }

  if (posterFind(name, &id) != OK) {
    genom_log_warn(1, "no such port %s", name);
    return genom_no_such_service(&self->control.context);
  }

  s = posterTake(id, POSTER_READ);
  if (s != OK) {
    genom_log_warn(1, "cannot access metadata port %s", name);
    return genom_no_such_service(&self->control.context);
  }

  data = posterAddr(id);
  m = -1;
  s =
    genom_deserialize_<"[[dotgen types ::pocolibs::metadata::component] mangle]">
    (&data, &m, meta);
  if (s) {
    genom_log_warn(1, "cannot read metadata port %s contents", name);
    return genom_no_such_service(&self->control.context);
  }
  posterGive(id);

  return 0;
}


<'foreach p [$component ports] {'>

/* <"[--- genom_${comp}_[$p name]_get_ph --------------------------------]"> */

static __inline__ struct genom_<"$comp">_<"[$p name]">_ph *
genom_<"$comp">_<"[$p name]">_get_ph(
<'  if {[$p kind] == "multiple"} {'>
  const char *id,
<'  }'>
  struct genom_<"$comp">_<"[$p name]">_port *p)
{
<'  if {[$p kind] == "multiple"} {'>
  int n;
  for(n = 0; n < p->nh; n++)
    if (p->h[n] && !strcmp(p->h[n]->name, id))
      return p->h[n];
  return NULL;
<'  } else {'>
  return &p->h;
<'  }'>
}


/* <"[--- genom_${comp}_[$p name]_data ----------------------------------]"> */

<"[[$p datatype] argument reference]">
genom_<"$comp">_<"[$p name]">_data(
<'  if {[$p kind] == "multiple"} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  struct genom_<"$comp">_<"[$p name]">_port *p;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;

  p = &self->data->self->ports.<"[$p name]">;

<'  if {[$p kind] == "multiple"} {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(id, p);
<'  } else {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(p);
<'  }'>
  if (!ph || !ph->id) return NULL;
  return <"[[$p datatype] pass reference {ph->buffer}]">;
}


/* <"[--- genom_${comp}_[$p name]_open ----------------------------------]"> */

genom_event
genom_<"$comp">_<"[$p name]">_open(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  struct genom_<"$comp">_<"[$p name]">_port *p;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;
  char name[H2_DEV_MAX_NAME];
<'  if {"out" in [$p dir]} {'>
  STATUS s;
  size_t l;
<'  }'>
  int n;

  p = &self->data->self->ports.<"[$p name]">;

<'  if {"multiple" in [$p kind]} {'>
  n = snprintf(name, sizeof(name), "%s/<"[$p name]">/%s", genom_instance, id);
<'  } else {'>
  n = snprintf(name, sizeof(name), "%s/<"[$p name]">", genom_instance);
<'  }'>
  if (n <= 0 || n >= sizeof(name)) {
    genom_log_warn(0, "port <"[$p name]"> name too long");
    genom_log_warn(0, "port name length limited to %zu characters",
                   H2_DEV_MAX_NAME - strlen(genom_instance) - 2);
    return genom_syserr(&(genom_syserr_detail){ .code = ENAMETOOLONG }, self);
  }

<'  if {"multiple" in [$p kind]} {'>
  for(n = 0; n < p->nh; n++)
    if (p->h[n] && !strcmp(p->h[n]->name, id))
      return genom_ok;
  for(n = 0; n < p->nh; n++)
    if (!p->h[n]) break;
  if (n >= p->nh) {
    struct genom_<"$comp">_<"[$p name]">_ph **pph;
    pph = realloc(p->h, (p->nh+1)*sizeof(*pph));
    if (!pph)
      return genom_syserr(&(genom_syserr_detail){ .code = ENOMEM }, self);
    p->h = pph;
    n = p->nh++;
  }
  p->h[n] = malloc(sizeof(*ph));
  if (!p->h[n])
    return genom_syserr(&(genom_syserr_detail){ .code = ENOMEM }, self);

  ph = p->h[n];
  genom_tinit_<"[[$p datatype] mangle]">(
    <"[[$p datatype] pass reference ph->buffer]">);
  strncpy(ph->name, id, sizeof(ph->name));
<'  } else {'>
  n = 0;
  ph = &p->h;
<'  }'>
  ph->id = NULL;

<'  if {"out" in [$p dir]} {'>
  l = genom_maxserialen_<"[[$p datatype] mangle]">();

  s = posterCreate(name, (int)l, &ph->id);
  if (s == ERROR) {
    genom_log_warn(1, "cannot create outport %s", name);
<'    if {"multiple" in [$p kind]} {'>
    free(ph);
    p->h[n] = NULL;
<'    }'>
    return genom_syserr(&(genom_syserr_detail){ .code = ENOENT }, self);
  }
  ph->size = l;
  genom_log_info("created outport %s", name);
<'  } else {'>
  genom_log_info("created inport %s", name);
<'  }'>
  return genom_ok;
}


/* <"[--- genom_${comp}_[$p name]_close ------------------------------------]"> */

genom_event
genom_<"$comp">_<"[$p name]">_close(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  struct genom_<"$comp">_<"[$p name]">_port *p;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;

  p = &self->data->self->ports.<"[$p name]">;

<'  if {[$p kind] == "multiple"} {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(id, p);
<'  } else {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(p);
<'  }'>
  if (!ph || !ph->id)
    return genom_syserr(&(genom_syserr_detail){ .code = ENOENT }, self);

<'  if {"out" in [$p dir]} {'>
  posterDelete(ph->id);
<'    if {"multiple" in [$p kind]} {'>
  genom_log_info("destroyed outport <"[$p name]">/%s", ph->name);
<'    } else {'>
  genom_log_info("destroyed outport <"[$p name]">");
<'    }'>
<'  } else {'>
<'    if {"multiple" in [$p kind]} {'>
  genom_log_info("destroyed inport <"[$p name]">/%s", ph->name);
<'    } else {'>
  genom_log_info("destroyed inport <"[$p name]">");
<'    }'>
<'  }'>
  ph->id = NULL;
  return genom_ok;
}


/* <"[--- genom_${comp}_[$p name]_delete -----------------------------------]"> */

void
genom_<"$comp">_<"[$p name]">_delete(genom_context self)
{
<'  if {"multiple" in [$p kind]} {'>
  struct genom_<"$comp">_<"[$p name]">_port *p;
  int n;

  p = &self->data->self->ports.<"[$p name]">;

  for(n = 0; n < p->nh; n++)
    if (p->h[n]) {
      genom_<"$comp">_<"[$p name]">_close(p->h[n]->name, self);
      p->h[n] = NULL;
    }
<'  } else {'>
  genom_<"$comp">_<"[$p name]">_close(self);
<'  }'>
}

<'}'>

<'foreach p [$component ports in] {'>

/* <"[--- genom_${comp}_[$p name]_connect -------------------------------]"> */

genom_event
genom_<"$comp">_<"[$p name]">_connect(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  const char *name, genom_context self)
{
  struct genom_<"$comp">_<"[$p name]">_port *p =
    &self->data->self->ports.<"[$p name]">;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;
  POSTER_ID pid;

<'  if {[$p kind] == "multiple"} {'>
  genom_event s = genom_<"$comp">_<"[$p name]">_open(id, self);
  if (s) return s;
  ph = genom_<"$comp">_<"[$p name]">_get_ph(id, p);
<'  } else {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(p);
<'  }'>

  if (!ph) return genom_no_such_inport(self);

  if (posterFind(name, &pid) != OK) {
    genom_log_warn(1, "no such port %s", name);
    return genom_no_such_outport(self);
  }
  if (posterTake(pid, POSTER_READ) != OK) {
    genom_log_warn(1, "cannot connect port %s", name);
    return genom_port_io(self);
  }
  posterGive(pid);

  ph->id = pid;
  return genom_ok;
}


/* <"[--- genom_${comp}_[$p name]_read ----------------------------------]"> */

genom_event
genom_<"$comp">_<"[$p name]">_read(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  struct genom_<"$comp">_<"[$p name]">_port *p;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;
  STATUS s;
  char *b;
  ssize_t max;

  p = &self->data->self->ports.<"[$p name]">;

<'  if {[$p kind] == "multiple"} {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(id, p);
<'  } else {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(p);
<'  }'>
  if (!ph || !ph->id) return genom_no_such_inport(self);

  s = posterTake(ph->id, POSTER_READ);
  if (s != OK) {
<'  if {"multiple" in [$p kind]} {'>
    genom_log_warn(1, "cannot access inport <"[$p name]">/%s", ph->name);
<'  } else {'>
    genom_log_warn(1, "cannot access inport <"[$p name]">");
<'  }'>
    return genom_port_io(self);
  }

  b = posterAddr(ph->id);
  max = -1;
  s = genom_deserialize_<"[[$p datatype] mangle]">(
    &b, &max, <"[[$p datatype] pass value ph->buffer]">);
  posterGive(ph->id);
  if (s) {
<'  if {"multiple" in [$p kind]} {'>
    genom_log_warn(0, "cannot read inport <"[$p name]">/%s contents", ph->name);
<'  } else {'>
    genom_log_warn(0, "cannot read inport <"[$p name]"> contents");
<'  }'>
    return genom_serialization(self);
  }

  return genom_ok;
}

<'}'>

<'foreach p [$component ports out] {'>

/* <"[--- genom_${comp}_[$p name]_write ---------------------------------]"> */

genom_event
genom_<"$comp">_<"[$p name]">_write(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  static struct genom_<"$comp">_<"[$p name]">_port *p;
  struct genom_<"$comp">_<"[$p name]">_ph *ph;
  STATUS s;
  char *b;
  size_t l;

  p = &self->data->self->ports.<"[$p name]">;

<'  if {[$p kind] == "multiple"} {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(id, p);
<'  } else {'>
  ph = genom_<"$comp">_<"[$p name]">_get_ph(p);
<'  }'>
  if (!ph || !ph->id) return genom_no_such_outport(self);

  l = genom_serialen_<"[[$p datatype] mangle]">(
    <"[[$p datatype] pass value ph->buffer]">);
  if (l > ph->size || l + (1<<17) < ph->size) {
    s = posterIoctl(ph->id, FIO_RESIZE, &l);
    if (s != OK) {
<'  if {"multiple" in [$p kind]} {'>
      genom_log_warn(1, "cannot resize outport <"[$p name]">/%s to %zu bytes",
                     ph->name, l);
<'  } else {'>
      genom_log_warn(1, "cannot resize outport <"[$p name]"> to %zu bytes", l);
<'  }'>
      return genom_serialization(self);
    }
    ph->size = l;
  }

  s = posterTake(ph->id, POSTER_WRITE);
  if (s != OK) {
<'  if {"multiple" in [$p kind]} {'>
    genom_log_warn(1, "cannot access outport <"[$p name]">/%s", ph->name);
<'  } else {'>
    genom_log_warn(1, "cannot access outport <"[$p name]">");
<'  }'>
    return genom_port_io(self);
  }

  b = posterAddr(ph->id);
  genom_serialize_<"[[$p datatype] mangle]">(
    &b, <"[[$p datatype] pass value ph->buffer]">);
  s = posterGive(ph->id);
  if (s != OK) {
<'  if {"multiple" in [$p kind]} {'>
    genom_log_warn(1, "cannot flush outport <"[$p name]">/%s", ph->name);
<'  } else {'>
    genom_log_warn(1, "cannot flush outport <"[$p name]">");
<'  }'>
    return genom_port_io(self);
  }

  return genom_ok;
}

<'}'>

/* --- end of file --------------------------------------------------------- */
