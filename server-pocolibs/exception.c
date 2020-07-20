<'
# Copyright (c) 2013-2014 LAAS/CNRS
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
#                                      Anthony Mallet on Fri Mar 22 2013
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <stdlib.h>
#include <string.h>

#include "<"$comp">_control_task.h"

genom_event
genom_pocolibs_raise(genom_event ex, void *detail, size_t size,
                     genom_context self)
{
  struct genom_context_data *data = self->data;

  if (data->exsize < size) {
    void *exdetail = realloc(data->exdetail, size);
    if (!exdetail) return genom_fatal;

    data->exsize = size;
    data->exdetail = exdetail;
  }
  data->ex = ex;
  memcpy(data->exdetail, detail, size);
  return ex;
}

const void *
genom_pocolibs_raised(genom_event *ex, genom_context self)
{
  struct genom_context_data *data = self->data;

  if (ex) *ex = data->ex;
  return data->exdetail;
}
