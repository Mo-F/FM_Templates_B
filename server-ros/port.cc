<'
# Copyright (c) 2012-2015 LAAS/CNRS
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
#                                      Anthony Mallet on Thu Feb 16 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include "<"$comp">_internals.h"

<'foreach p [$component ports in] {'>

/* <"[--- Port [$p name] ------------------------------------------------]"> */

void
genom_port_<"[$p name]">_base::datacb(
  const boost::shared_ptr<genom::port_<"[$p name]"> const> &d) {
  pthread_spin_lock(&lock);
  latest = d;
  pthread_spin_unlock(&lock);
}

<"[[$p datatype] argument reference]">
genom_<"$comp">_<"[$p name]">_data(
<'  if {[$p kind] == "multiple"} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  genom_port_<"[$p name]">_base *h;

<'  if {[$p kind] == "multiple"} {'>
  h = self->data->self->ports.<"[$p name]">_.find(id);
<'  } else {'>
  h = &self->data->self->ports.<"[$p name]">_;
<'  }'>
  if (!h || !h->data) return NULL;
  return const_cast< <"[[$p datatype] argument reference]"> >(
    <"[[$p datatype] pass reference h->data->v]">);
}

genom_event
genom_<"$comp">_<"[$p name]">_read(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  genom_port_<"[$p name]">_base *h;

<'  if {[$p kind] == "multiple"} {'>
  h = self->data->self->ports.<"[$p name]">_.find(id);
  if (!h) {
    genom_syserr_detail d = { ENOENT };
    return genom_syserr(&d, self);
  }
<'  } else {'>
  h = &self->data->self->ports.<"[$p name]">_;
<'  }'>
  pthread_spin_lock(&h->lock);
  h->data = h->latest;
  pthread_spin_unlock(&h->lock);
  return genom_ok;
}

<'}'>

<'foreach p [$component ports out] {'>

/* <"[--- Port [$p name] ------------------------------------------------]"> */

<"[[$p datatype] argument reference]">
genom_<"$comp">_<"[$p name]">_data(
<'  if {[$p kind] == "multiple"} {'>
  const char *id,
<'  }'>
  genom_context self)
{
  genom_port_<"[$p name]">_base *b;

<'  if {[$p kind] == "multiple"} {'>
  b = self->data->self->ports.<"[$p name]">_.find(id);
<'  } else {'>
  b = &self->data->self->ports.<"[$p name]">_;
<'  }'>
  if (!b) return NULL;
  return <"[[$p datatype] pass reference b->data->v]">;
}

genom_event
genom_<"$comp">_<"[$p name]">_write(
<'  if {"multiple" in [$p kind]} {'>
  const char *id,
<'  }'>
  genom_context self)
{
<'  if {[$p kind] == "multiple"} {'>
  self->data->self->ports.<"[$p name]">_.publish(id);
<'  } else {'>
  self->data->self->ports.<"[$p name]">_.publish();
<'  }'>
  return genom_ok;
}

<'  if {"multiple" in [$p kind]} {'>
genom_event
genom_<"$comp">_<"[$p name]">_open(const char *id, genom_context self)
{
  int e = self->data->self->ports.<"[$p name]">_.open(
    self->data->self->control.node, id);
  if (!e) return genom_ok;

  genom_syserr_detail d = { e };
  return genom_syserr(&d, self);
}

genom_event
genom_<"$comp">_<"[$p name]">_close(const char *id, genom_context self)
{
  self->data->self->ports.<"[$p name]">_.close(id);
  return genom_ok;
}
<'  }'>

<'}'>


/* --- state port ---------------------------------------------------------- */

int
genom_<"$comp">_component_data::genom_state_init(void)
{
  genom_state_component *data;

  data = ports.genom_state_.handle.data(&control.context);

  /* task list */
<'set tasks [$component tasks]'>
  if (genom_sequence_reserve(&data->task, <"[llength $tasks]">))
    return ENOMEM;
<'foreach t $tasks {'>
  {
    genom_state_task *task;
    task = &data->task._buffer[<"${comp}_[$t name]_TASKID">];

    snprintf(task->name, sizeof(task->name), "%s", "<"[$t name]">");
    task->rusage.cycles = 0;
    task->rusage.timings.last = 0;
    task->rusage.timings.max = 0;
    task->rusage.timings.avg = 0;
    task->rusage.load.last = 0;
    task->rusage.load.max = 0;
    task->rusage.load.avg = 0;
    task->activity._length = 0;
    if (genom_sequence_reserve(&task->activity,
                               genom_<"$comp">_activities::MAX_ACTIVITIES))
      return ENOMEM;
  }
<'}'>
  data->task._length = <"[llength $tasks]">;

  /* automatic versioning - nice idea borrowed from ROS */
  snprintf(data->digest, sizeof(data->digest),
           "%s", "<"[$component digest]">");
  snprintf(data->date, sizeof(data->date),
           "%s", "<"[clock format [clock sec]]">");
  snprintf(data->version, sizeof(data->version),
           "%s", "<"[$component version]">");

  ports.genom_state_.handle.write(&control.context);

  genom_<"$comp">_log_info("initialized outport genom_state");
  return 0;
}

int
genom_<"$comp">_component_data::genom_state_update(struct genom_<"$comp">_exec_task *task)
{
  genom_state_component *data;
  genom_state_task *t;
  genom_<"$comp">_activity *a;
  const char *s;
  size_t i, j;

  data = ports.genom_state_.handle.data(&control.context);
  t = &data->task._buffer[task->num];

  for(i = j = 0; i < genom_<"$comp">_activities::MAX_ACTIVITIES; i++) {
    a = task->activities.a[i];
    if (!a) continue;
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

int
genom_<"$comp">_component_data::genom_state_refresh()
{
<'if {[llength [$component tasks]]} {'>
  genom_state_component *data;
  genom_state_task *task;

  data = ports.genom_state_.handle.data(&control.context);
<'}'>

  /* task rusage */
<'foreach t [$component tasks] {'>
  task = &data->task._buffer[<"${comp}_[$t name]_TASKID">];
  //  pthread_spin_lock(&tasks.<"[$t name]">_.rlock);
  task->rusage = tasks.<"[$t name]">_.rusage;
  //  pthread_spin_unlock(&tasks.<"[$t name]">_.rlock);
<'}'>

<'if {[llength [$component tasks]]} {'>
  ports.genom_state_.handle.write(&control.context);
<'}'>
  return 0;
}
