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
#                                      Anthony Mallet on Wed Mar  7 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]

lang c
'>
#ifndef H_<"$COMP">_PORTLIB
#define H_<"$COMP">_PORTLIB

#include <stddef.h>

#include "h2devLib.h"
#include "posterLib.h"

#include "<"$comp">_c_types.h"
#include "typecopy.h"

/* forward declaration */
struct genom_component_data;
struct genom_activities;


/* --- builtin ports ------------------------------------------------------- */

/* state */
int	genom_state_<"$comp">_init(struct genom_component_data *self);
int	genom_state_<"$comp">_update(struct genom_component_data *self,
		struct genom_activities *activities, int tnum);
genom_event
	genom_state_<"$comp">_refresh(struct genom_component_data *self);

/* metadata */
int		genom_metadata_<"$comp">_init(
			struct genom_component_data *self);
genom_event	genom_metadata_<"$comp">_fetch(
			struct genom_component_data *self,
                        const char *comp, pocolibs_metadata_component *meta);


/* === Port methods ======================================================== */

<'foreach p [$component ports] {'>
<'  switch -- [$p kind] {'>
<'    simple {'>

<"[[$p datatype] argument reference]">
		genom_<"$comp">_<"[$p name]">_data(genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_open(genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_close(genom_context self);
void		genom_<"$comp">_<"[$p name]">_delete(genom_context self);

<'      switch -- [$p dir] {'>
<'        in {'>
genom_event	genom_<"$comp">_<"[$p name]">_connect(
                  const char *name, genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_read(genom_context self);
<'        }'>
<'        out {'>
genom_event	genom_<"$comp">_<"[$p name]">_write(genom_context self);
<'        }'>
<'      }'>

<'    }'>
<'    multiple {'>

<"[[$p datatype] argument reference]">
		genom_<"$comp">_<"[$p name]">_data(const char *id,
                             genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_open(const char *id,
                        genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_close(const char *id,
                        genom_context self);
void		genom_<"$comp">_<"[$p name]">_delete(genom_context self);

<'      switch -- [$p dir] {'>
<'        in {'>
genom_event	genom_<"$comp">_<"[$p name]">_connect(const char *id,
			const char *name, genom_context self);
genom_event	genom_<"$comp">_<"[$p name]">_read(const char *id,
                        genom_context self);
<'        }'>
<'        out {'>
genom_event	genom_<"$comp">_<"[$p name]">_write(const char *id,
                        genom_context self);
<'        }'>
<'      }'>

<'    }'>
<'    default { error "unknown port kind" }'>
<'  }'>
<'}'>


/* === Port structures ===================================================== */

<'foreach p [$component ports] {'>

/* <"[--- [$p name] -----------------------------------------------------]"> */

struct genom_<"$comp">_<"[$p name]">_port {
  <"[[$p type] declarator handle]">;

  struct genom_<"$comp">_<"[$p name]">_ph {
    POSTER_ID id;
<'  if {"multiple" in [$p kind]} {'>
    char name[H2_DEV_MAX_NAME];
<'  }'>
<'  if {"out" in [$p dir]} {'>
    size_t size;
<'  }'>
    <"[[$p datatype] declarator buffer]">;
  } <"[expr {("multiple" in [$p kind]) ? "**" : ""}]"> h;
<'  if {"multiple" in [$p kind]} {'>
  int nh;
<'  }'>
};

/* initializer */
static __inline__ void
genom_tinit_<"$comp">_<"[$p name]">_port(
  struct genom_<"$comp">_<"[$p name]">_port *p)
{
  p->handle.data = genom_<"$comp">_<"[$p name]">_data;
<'  switch -- [$p kind]/[$p dir] {'>
<'    simple/in {'>
  p->handle.read = genom_<"$comp">_<"[$p name]">_read;
  p->h.id = NULL;
  genom_tinit_<"[[$p datatype] mangle]">(
    <"[[$p datatype] pass reference p->h.buffer]">);
<'    }'>
<'    simple/out {'>
  p->handle.write = genom_<"$comp">_<"[$p name]">_write;
  p->h.id = NULL;
  p->h.size = 0;
  genom_tinit_<"[[$p datatype] mangle]">(
    <"[[$p datatype] pass reference p->h.buffer]">);
<'    }'>
<'    multiple/in {'>
  p->handle.read = genom_<"$comp">_<"[$p name]">_read;
  p->h = NULL;
  p->nh = 0;
<'    }'>
<'    multiple/out {'>
  p->handle.write = genom_<"$comp">_<"[$p name]">_write;
  p->handle.open = genom_<"$comp">_<"[$p name]">_open;
  p->handle.close = genom_<"$comp">_<"[$p name]">_close;
  p->h = NULL;
  p->nh = 0;
<'    }'>
<'  }'>
}

/* finalizer */
static __inline__ void
genom_tfini_<"$comp">_<"[$p name]">_port(
  struct genom_<"$comp">_<"[$p name]">_port *p)
{
<'  if {"multiple" in [$p kind]} {'>
  if (p->h) free(p->h);
<'  } else {'>
  genom_tfini_<"[[$p datatype] mangle]">(
    <"[[$p datatype] pass reference p->h.buffer]">);
<'  }'>
}

<'}'>

#endif /* H_<"$COMP">_PORTLIB */
