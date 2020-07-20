<'
# Copyright (c) 2011-2013 LAAS/CNRS
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
#                                      Anthony Mallet on Fri Mar  2 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

lang c
'>
#include "autoconf/acheader.h"

#include <assert.h>
#include <stddef.h>

#include "portLib.h"
#include "errnoLib.h"
#include "gcomLib.h"

#include "<"$comp">_parameters.h"
#include "<"$comp">_msglib.h"

<'foreach s [$component services] {'>

/* <"[--- Service [$s name] encoding/decoding ---------------------------]"> */

int
genom_<"$comp">_<"[$s name]">_encode(
  char *buffer, int size, char *dst, int maxsize)
{
<'  if {[llength [$s parameters inout out]]} {'>
  char *p = dst;
  struct genom_<"$comp">_<"[$s name]">_output *out =
    (struct genom_<"$comp">_<"[$s name]">_output *)buffer;
  size_t s;

  s = 1;
<'    foreach p [$s parameters inout out] {'>
  s += genom_serialen_<"[[$p type] mangle]">(
    <"[[$p type] pass value out->[$p name]]">);
<'    }'>
  if (s > maxsize) return ERROR;

  *p++ = *"";
<'    foreach p [$s parameters inout out] {'>
  genom_serialize_<"[[$p type] mangle]">(
    &p, <"[[$p type] pass value out->[$p name]]">);
<'    }'>

  return s;
<'  } else {'>
  return 0;
<'  }'>
}

int
genom_<"$comp">_<"[$s name]">_decode(
  char *buffer, int size, char *dst, int maxsize)
{
<'  if {[llength [$s parameters in inout]]} {'>
  ssize_t len = size;
  char *p = buffer;
  struct genom_<"$comp">_<"[$s name]">_input *in =
    (struct genom_<"$comp">_<"[$s name]">_input *)dst;
  int s;
  if (size == 0) return 0;

<'  foreach p [$s parameters in inout] {'>
  s = genom_deserialize_<"[[$p type] mangle]">(
    &p, &len, <"[[$p type] pass reference in->[$p name]]">);
  if (s) {
    switch(s) {
      case ENOMSG: errnoSet(S_gcomLib_SMALL_DATA_STR);
      case ENOMEM: errnoSet(S_gcomLib_MALLOC_FAILED);
      default: errnoSet(S_gcomLib_NOT_A_LETTER);
    }
    return ERROR;
  }
<'  }'>

  if (len != 0) {
    errnoSet(S_gcomLib_SMALL_LETTER);
    return ERROR;
  }
  return size;
<'  } else {'>
  return 0;
<'  }'>
}

<'}'>

<'foreach e [$component throws] {'>

/* <"[--- Exception [$e name] encoding ----------------------------------]"> */

int
genom_<"$comp">_<"[$e cname]">_encodex(
  char *buffer, int size, char *dst, int maxsize)
{
  <"[$e declarator *out]"> = (<"[$e declarator *]">)buffer;
  char *p = dst;
  size_t s;

  s = sizeof(<"[$e cname]">_id);
  s += genom_serialen_<"[$e mangle]">(out);
  if (s > maxsize) return ERROR;

  memcpy(p, <"[$e cname]">_id, sizeof(<"[$e cname]">_id));
  p += sizeof(<"[$e cname]">_id);
  genom_serialize_<"[$e mangle]">(&p, out);
  return s;
}

<'}'>

/* --- Exception genom::unkex encoding ------------------------------------- */

int
genom_<"$comp">_genom_unkex_encodex(
  char *buffer, int size, char *dst, int maxsize)
{
  genom_unkex_detail *out = (genom_unkex_detail *)buffer;
  size_t s;

  s = sizeof(genom_unkex_id);
  s += sizeof(*out);
  if (s > maxsize) return ERROR;

  memcpy(dst, genom_unkex_id, sizeof(genom_unkex_id));
  dst += sizeof(genom_unkex_id);
  memcpy(dst, out, sizeof(*out));
  return s;
}


/* --- Exception genom::syserr encoding ------------------------------------ */

int
genom_<"$comp">_genom_syserr_encodex(
  char *buffer, int size, char *dst, int maxsize)
{
  genom_syserr_detail *out = (genom_syserr_detail *)buffer;
  size_t s;

  s = sizeof(genom_syserr_id);
  s += sizeof(*out);
  if (s > maxsize) return ERROR;

  memcpy(dst, genom_syserr_id, sizeof(genom_syserr_id));
  dst += sizeof(genom_syserr_id);
  memcpy(dst, out, sizeof(*out));
  return s;
}
