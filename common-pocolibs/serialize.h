<'
#
# Copyright (c) 2012-2014,2017 LAAS/CNRS
# All rights reserved.
#
# Redistribution  and  use  in  source  and binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of  source  code must retain the  above copyright
#      notice and this list of conditions.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice and  this list of  conditions in the  documentation and/or
#      other materials provided with the distribution.
#
# THE SOFTWARE  IS PROVIDED "AS IS"  AND THE AUTHOR  DISCLAIMS ALL WARRANTIES
# WITH  REGARD   TO  THIS  SOFTWARE  INCLUDING  ALL   IMPLIED  WARRANTIES  OF
# MERCHANTABILITY AND  FITNESS.  IN NO EVENT  SHALL THE AUTHOR  BE LIABLE FOR
# ANY  SPECIAL, DIRECT,  INDIRECT, OR  CONSEQUENTIAL DAMAGES  OR  ANY DAMAGES
# WHATSOEVER  RESULTING FROM  LOSS OF  USE, DATA  OR PROFITS,  WHETHER  IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR  OTHER TORTIOUS ACTION, ARISING OUT OF OR
# IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#                                           Anthony Mallet on Mon Feb 27 2012
#

# check arguments
if {[llength $argv] != 1} { error "expected arguments: types" }
lassign $argv types
lang c
'>
#ifndef H_GENOM_SERIALIZE
#define H_GENOM_SERIALIZE

#ifdef __linux__
/* Apparently we need _GNU_SOURCE defined to get access to strnlen on Linux */
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
#endif

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

<'foreach t $types {'>
static __inline__ size_t	genom_maxserialen_<"[$t mangle]">(void);
static __inline__ size_t	genom_serialen_<"[$t mangle]">(
					<"[$t argument value]">);
static __inline__ void		genom_serialize_<"[$t mangle]">(char **,
					<"[$t argument value]">);
static __inline__ int		genom_deserialize_<"[$t mangle]">(char **,
					ssize_t *, <"[$t argument reference]">);
<'}'>


<'foreach t $types {'>

/* <"[--- [$t mangle] ===================================================]"> */

static __inline__ size_t
genom_maxserialen_<"[$t mangle]">(void)
{
<'  if {![$t fixed]} {'>
<'    if {[$t kind] == "native"} {'>
  return 0;
<'    } else {'>
  /* maximum request size for variable types */

<'foreach c [dotgen components] {
set comp [$c name]'>

  extern size_t <"$comp">_varmsg_maxsize;
  return <"$comp">_varmsg_maxsize;
<'}'>

<'    }'>
<'  } else {'>
<'
    switch -glob -- [$t kind] {
      typedef - {* member} {'>
  return genom_maxserialen_<"[[$t type] mangle]">();
<'      }'>
<'      sequence {'>
  return sizeof(uint32_t) +
    <"[$t length]"> * sizeof(<"[[$t type] declarator]">);
<'      }'>
<'      optional {'>
  return sizeof(uint32_t) + sizeof(<"[[$t type] declarator]">);
<'      }'>
<'      array {'>
  return <"[$t length]"> * sizeof(<"[[$t type] declarator]">);
<'
      }
      string {'>
  return <"[$t length]">;
<'
      }
      exception {'>
<'        if {[llength [$t members]]} {'>
  return sizeof(<"[$t declarator]">);
<'        } else {'>
  return 0;
<'        }'>
<'
      }
      default {'>
  return sizeof(<"[$t declarator]">);
<'
      }
    }'>
<'  }'>
}

static __inline__ size_t
genom_serialen_<"[$t mangle]">(<"[$t argument value data]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+native {'>
  (void)data; /* fix -Wunused-parameter */
  return 0;
<'    }'>
<'    *+typedef - {*+* member} {'>
  return genom_serialen_<"[[$t type] mangle]">(data);
<'    }'>
<'    fixed+sequence {'>
  return sizeof(data->_length) + data->_length * sizeof(*data->_buffer);
<'    }'>
<'    fixed+optional {'>
  return sizeof(uint32_t) + (data->_present?1:0) * sizeof(data->_value);
<'    }'>
<'    fixed+array {'>
  return <"[$t length]"> * sizeof(*data);
<'
    }
    fixed+string {'>
  (void)data; /* fix -Wunused-parameter */
  return <"[$t length]">;
<'
    }
    fixed+exception {'>
<'      if {[llength [$t members]]} {'>
  return sizeof(*data);
<'        } else {'>
  (void)data; /* fix -Wunused-parameter */
  return 0;
<'        }'>
<'    }'>
<'    fixed+* {'>
  return sizeof(<"[$t dereference value data]">);
<'    }'>
<'    variable+struct - variable+exception {'>
<'      if {[llength [$t members]]} {'>
  size_t s = 0;
<'        foreach e [$t members] {'>
  s += genom_serialen_<"[$e mangle]">(
    <"[$e pass value data->[$e name]]">);
<'        }'>
  return s;
<'      } else {'>
  (void)data; /* fix -Wunused-parameter */
  return 0;
<'      }'>
<'    }'>
<'    variable+union {'>
  size_t s = 0, e;
<'        foreach e [$t members] {'>
  e = genom_serialen_<"[$e mangle]">(
    <"[$e pass value data->_u.[$e name]]">);
  if (s < e) s = e;
<'        }'>
  s += genom_serialen_<"[[$t discr] mangle]">(
    <"[[$t discr] pass value data->_d]">);
  return s;
<'    }'>
<'    variable+array {'>
  uint32_t i;
  size_t s = 0;
  for (i=0; i<<"[$t length]">; i++)
    s += genom_serialen_<"[[$t type] mangle]">(
      <"[[$t type] pass value {data[i]}]">);
  return s;
<'
    }
    variable+string {'>
  if (!data) return 1;
  return 1 + strlen(data);
<'    }'>
<'    variable+sequence {'>
  uint32_t i;
  size_t s = sizeof(data->_length);
<'      if {![catch {$t length} l]} {'>
  assert(data->_length <= <"$l">);
<'      }'>
  for (i=0; i<data->_length; i++)
    s += genom_serialen_<"[[$t type] mangle]">(
      <"[[$t type] pass value {data->_buffer[i]}]">);
  return s;
<'    }'>
<'    variable+optional {'>
  size_t s = sizeof(uint32_t);
  if (data->_present)
    s += genom_serialen_<"[[$t type] mangle]">(
      <"[$e pass value data->_value]">);
  return s;
<'    }'>
<'      default {
      error "reached unreachable"
    }
  }'>
}

static __inline__ void
genom_serialize_<"[$t mangle]">(char **buffer, <"[$t argument value data]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+native {'>
  (void)buffer; (void)data; /* fix -Wunused-parameter */
  return;
<'    }'>
<'    *+typedef - {*+* member} {'>
  return genom_serialize_<"[[$t type] mangle]">(buffer, data);
<'    }'>
<'    *+optional {'>
  uint32_t p = data->_present ? 1 : 0;
  memcpy(*buffer, &p, sizeof(p));
  *buffer += sizeof(p);
  if (p)
    genom_serialize_<"[[$t type] mangle]">(
      buffer, <"[[$t type] pass value {data->_value}]">);
<'    }'>
<'    fixed+sequence {'>
  assert(data->_length <= <"[$t length]">);
  memcpy(*buffer, &data->_length, sizeof(data->_length));
  *buffer += sizeof(data->_length);
  memcpy(*buffer, data->_buffer, data->_length * sizeof(*data->_buffer));
  *buffer += data->_length * sizeof(*data->_buffer);
<'    }'>
<'    fixed+array {'>
  memcpy(*buffer, data, <"[$t length]"> * sizeof(*data));
  *buffer += <"[$t length]"> * sizeof(*data);
<'
    }
    fixed+string {'>
  memcpy(*buffer, data, <"[$t length]">);
  *buffer += <"[$t length]">;
<'    }'>
<'    fixed+exception {'>
<'      if {[llength [$t members]]} {'>
  memcpy(*buffer, data, sizeof(*data));
  *buffer += sizeof(*data);
<'      } else {'>
  (void)buffer; (void)data; /* fix -Wunused-parameter */
<'      }'>
<'    }'>
<'    fixed+* {'>
  memcpy(*buffer, <"[$t address [$t dereference value data]]">,
         sizeof(<"[$t dereference value data]">));
  *buffer += sizeof(<"[$t dereference value data]">);
<'    }'>
<'    variable+struct - variable+exception {'>
<'      if {![llength [$t members]]} {'>
  (void)buffer; (void)data; /* fix -Wunused-parameter */
<'      }'>
<'      foreach e [$t members] {'>
  genom_serialize_<"[$e mangle]">(
    buffer, <"[$e pass value data->[$e name]]">);
<'      }'>
<'    }'>
<'    variable+union {'>
  genom_serialize_<"[[$t discr] mangle]">(
    buffer, <"[[$t discr] pass value data->_d]">);
  switch(data->_d) {
<'      foreach e [$t members] {'>
<'        foreach v [$e value] {'>
<'          if {$v != ""} {'>
    case <"[language::cname $v]">:
<'          } else {'>
    default:
<'          }'>
<'        }'>
      genom_serialize_<"[$e mangle]">(
        buffer, <"[$e pass value data->_u.[$e name]]">);
      break;
<'      }'>
  }
<'    }'>
<'    variable+array {'>
  uint32_t i;
  for (i=0; i<<"[$t length]">; i++)
    genom_serialize_<"[[$t type] mangle]">(
      buffer, <"[[$t type] pass value {data[i]}]">);
<'
    }
    variable+string {'>
  if (!data)
    *(*buffer)++ = '\0';
  else
    *buffer = 1 + stpcpy(*buffer, data);
<'    }'>
<'    variable+sequence {'>
  uint32_t i;
<'      if {![catch {$t length} l]} {'>
  assert(data->_length <= <"$l">);
<'      }'>
  memcpy(*buffer, &data->_length, sizeof(data->_length));
  *buffer += sizeof(data->_length);
  for (i=0; i<data->_length; i++)
    genom_serialize_<"[[$t type] mangle]">(
      buffer, <"[[$t type] pass value {data->_buffer[i]}]">);
<'    }'>
<'    default {
      error "reached unreachable"
    }
  }'>
}

static __inline__ int
genom_deserialize_<"[$t mangle]">(char **buffer, ssize_t *size,
  <"[$t argument reference data]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+native {'>
  (void)buffer; (void)size; (void)data; /* fix -Wunused-parameter */
<'    }'>
<'    *+typedef - {*+* member} {'>
  return genom_deserialize_<"[[$t type] mangle]">(buffer, size, data);
<'    }'>
<'    *+optional {'>
  uint32_t p;
  if (*size != -1) {
    *size -= sizeof(p);
    if (*size < 0) return ENOMSG;
  }
  p = *(uint32_t *)*buffer;
  *buffer += sizeof(p);
  data->_present = p?1:0;
  if (data->_present)
    genom_deserialize_<"[[$t type] mangle]">(
      buffer, size, <"[[$t type] pass reference {data->_value}]">);
<'    }'>
<'    fixed+sequence {'>
  if (*size != -1) {
    *size -= sizeof(data->_length);
    if (*size < 0) return ENOMSG;
  }
  data->_length = *(uint32_t *)*buffer;
  *buffer += sizeof(data->_length);
  assert(data->_length <= <"[$t length]">);
  if (*size != -1) {
    *size -= data->_length * sizeof(*data->_buffer);
    if (*size < 0) return ENOMSG;
  }
  memcpy(data->_buffer, *buffer, data->_length * sizeof(*data->_buffer));
  *buffer += data->_length * sizeof(*data->_buffer);
<'
    }
    fixed+array {'>
  if (*size != -1) {
    *size -= <"[$t length]"> * sizeof(*data);
    if (*size < 0) return ENOMSG;
  }
  memcpy(data, *buffer, <"[$t length]"> * sizeof(*data));
  *buffer += <"[$t length]"> * sizeof(*data);
<'
    }
    fixed+string {'>
  if (*size != -1) {
    *size -= <"[$t length]">;
    if (*size < 0) return ENOMSG;
  }
  memcpy(data, *buffer, <"[$t length]">);
  *buffer += <"[$t length]">;
<'    }'>
<'    fixed+exception {'>
<'      if {[llength [$t members]]} {'>
  if (*size != -1) {
    *size -= sizeof(*data);
    if (*size < 0) return ENOMSG;
  }
  memcpy(data, *buffer, sizeof(*data));
  *buffer += sizeof(*data);
<'      } else {'>
  (void)buffer; (void)size; (void)data; /* fix -Wunused-parameter */
<'      }'>
<'    }'>
<'    fixed+* {'>
  if (*size != -1) {
    *size -= sizeof(<"[$t dereference reference data]">);
    if (*size < 0) return ENOMSG;
  }
  memcpy(<"[$t address [$t dereference reference data]]">, *buffer,
         sizeof(<"[$t dereference reference data]">));
  *buffer += sizeof(<"[$t dereference reference data]">);
<'    }'>
<'    variable+struct - variable+exception {'>
<'      if {[llength [$t members]]} {'>
  int s;
<'        foreach e [$t members] {'>
  s = genom_deserialize_<"[$e mangle]">(
    buffer, size, <"[$e pass reference data->[$e name]]">);
  if (s) return s;
<'        }'>
<'      } else {'>
  (void)buffer; (void)size; (void)data; /* fix -Wunused-parameter */
<'      }'>
<'    }'>
<'    variable+union {'>
  int s;
  s = genom_deserialize_<"[[$t discr] mangle]">(
    buffer, size, <"[[$t discr] pass reference data->_d]">);
  if (s) return s;
  switch(data->_d) {
<'      foreach e [$t members] {'>
<'        foreach v [$e value] {'>
<'          if {$v != ""} {'>
    case <"[language::cname $v]">:
<'          } else {'>
    default:
<'          }'>
<'        }'>
      s = genom_deserialize_<"[$e mangle]">(
        buffer, size, <"[$e pass reference data->[$e name]]">);
      if (s) return s;
      break;
<'      }'>
  }
<'    }'>
<'    variable+array {'>
  uint32_t i;
  int s;
  for (i=0; i<<"[$t length]">; i++) {
    s = genom_deserialize_<"[[$t type] mangle]">(
      buffer, size, <"[[$t type] pass reference {data[i]}]">);
    if (s) return s;
  }
<'
    }
    variable+string {'>
  size_t l = (*size) ? strnlen(*buffer, *size) : strlen(*buffer);
  if (*size != -1) {
    *size -= l+1;
    if (*size < 0) return ENOMSG;
  }
  *data = realloc(*data, 1 + l);
  if (!*data) return ENOMEM;
  strcpy(*data, *buffer);
  *buffer += 1 + l;
<'    }'>
<'    variable+sequence {'>
  uint32_t i;
  if (*size != -1) {
    *size -= sizeof(i);
    if (*size < 0) return ENOMSG;
  }
  i = *(uint32_t *)*buffer;
<'      if {![catch {$t length} l]} {'>
  assert(i <= <"$l">);
<'      }'>
  if (genom_sequence_reserve(data, i)) return ENOMEM;
  *buffer += sizeof(data->_length);
  data->_length = i;
  for (i=0; i<data->_length; i++)
    genom_deserialize_<"[[$t type] mangle]">(
      buffer, size, <"[[$t type] pass reference {data->_buffer[i]}]">);
<'    }'>
<'    default {
      error "reached unreachable"
    }
  }'>
  return 0;
}
<'}'>

#endif /* H_GENOM_SERIALIZE */
