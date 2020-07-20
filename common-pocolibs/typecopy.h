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
#                                           Anthony Mallet on Wed Feb 29 2012
#

# check arguments
if {[llength $argv] != 1} { error "expected arguments: types" }
lassign $argv types
lang c
'>
#ifndef H_GENOM_TYPECOPY
#define H_GENOM_TYPECOPY

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

<'foreach t $types {'>
static __inline__ void	genom_tinit_<"[$t mangle]">(
				<"[$t argument reference]">);
static __inline__ void	genom_tfini_<"[$t mangle]">(
				<"[$t argument reference]">);
static __inline__ int	genom_tcopy_<"[$t mangle]">(
				<"[$t argument reference]">,
				<"[$t argument value]">);
<'}'>


<'foreach t $types {'>

/* <"[--- [$t mangle] ===================================================]"> */

static __inline__ void
genom_tinit_<"[$t mangle]">(<"[$t argument reference data]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+native {'>
  *data = NULL;
<'    }'>
<'    *+typedef - {*+* member} {'>
  genom_tinit_<"[[$t type] mangle]">(data);
<'
    }
    variable+sequence {'>
  data->_maximum = data->_length = 0;
  data->_buffer = NULL;
  data->_release = NULL;
<'
    }
    fixed+sequence {'>
  *(uint32_t *)&data->_maximum = <"[$t length]">;
  data->_length = 0;
<'
    }
    variable+string {'>
  *data = NULL;
<'
    }
    fixed+string {'>
  *data = '\0';
<'    }'>
<'    *+struct - *+exception {'>
<'      if {![llength [$t members]]} {'>
  (void)data; /* fix -Wunused-parameter */
<'      }'>
<'      foreach e [$t members] {'>
  genom_tinit_<"[$e mangle]">(<"[$e pass reference data->[$e name]]">);
<'      }'>
<'    }'>
<'    *+union {'>
<'      foreach first [$t members] {'>
<'        foreach v [$first value] { if {$v != ""} break }'>
<'        if {$v != ""} break'>
<'      }'>
  genom_tinit_<"[[$t disc] mangle]">(<"[[$t disc] pass reference data->_d]">);
<'      if {$v != ""} {'>
  data->_d = <"[language::cname $v]">;
<'      }'>
  genom_tinit_<"[$first mangle]">(
    <"[$first pass reference data->_u.[$first name]]">);
<'    }'>
<'    *+optional {'>
  genom_tinit_<"[[$t type] mangle]">(<"[[$t type] pass reference data->_value]">);
<'    }'>
<'    *+array {'>
  uint32_t i;
  for (i=0; i<<"[$t length]">; i++)
    genom_tinit_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {data[i]}]">);
<'    }'>
<'    default {'>
  (void)data; /* fix -Wunused-parameter */
<'    }'>
<'  }'>
}

static __inline__ void
genom_tfini_<"[$t mangle]">(<"[$t argument reference data]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+typedef - {*+* member} {'>
  genom_tfini_<"[[$t type] mangle]">(data);
<'
    }
    variable+sequence {'>
  uint32_t i;
  for (i=0; i<data->_length; i++)
    genom_tfini_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {data->_buffer[i]}]">);
  data->_maximum = data->_length = 0;
  if (data->_release && data->_buffer) data->_release(data->_buffer);
  data->_buffer = NULL;
  data->_release = NULL;
<'
    }
    fixed+sequence {'>
  uint32_t i;
  for (i=0; i<data->_length; i++)
    genom_tfini_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {data->_buffer[i]}]">);
<'
    }
    variable+string {'>
  if (*data) {
    free(*data);
    *data = NULL;
  }
<'    }'>
<'    *+struct - *+exception {'>
<'      if {![llength [$t members]]} {'>
  (void)data; /* fix -Wunused-parameter */
<'      }'>
<'      foreach e [$t members] {'>
  genom_tfini_<"[$e mangle]">(<"[$e pass reference data->[$e name]]">);
<'      }'>
<'    }'>
<'    *+union {'>
  switch(data->_d) {
<'      foreach e [$t members] {'>
<'        foreach v [$e value] {'>
<'          if {$v != ""} {'>
    case <"[language::cname $v]">:
<'          } else {'>
    default:
<'          }'>
<'        }'>
      genom_tfini_<"[$e mangle]">(<"[$e pass reference data->_u.[$e name]]">);
      break;
<'      }'>
  }
  genom_tfini_<"[[$t disc] mangle]">(<"[[$t disc] pass reference data->_d]">);
<'    }'>
<'    *+optional {'>
  genom_tfini_<"[[$t type] mangle]">(<"[[$t type] pass reference data->_value]">);
<'    }'>
<'    *+array {'>
  uint32_t i;
  for (i=0; i<<"[$t length]">; i++)
    genom_tfini_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {data[i]}]">);
<'    }'>
<'    default {'>
  (void)data; /* fix -Wunused-parameter */
<'    }'>
<'  }'>
}

static __inline__ int
genom_tcopy_<"[$t mangle]">(<"[$t argument reference dst]">,
  <"[$t argument value src]">)
{
<'
  set fixed [expr {[$t fixed]?"fixed":"variable"}]
  switch -glob -- $fixed+[$t kind] {'>
<'    *+native {'>
  *dst = (<"[$t declarator]">)src;
  return 0;
<'    }'>
<'    *+typedef - {*+* member} {'>
  return genom_tcopy_<"[[$t type] mangle]">(dst, src);
<'
    }
    fixed+sequence {'>
  assert(src->_length <= <"[$t length]">);
  dst->_length = src->_length;
  memcpy(dst->_buffer, src->_buffer, src->_length * sizeof(*src->_buffer));
  return 0;
<'
    }
    fixed+array {'>
  memcpy(dst, src, <"[$t length]"> * sizeof(*src));
  return 0;
<'    }'>
<'    fixed+string {'>
  strncpy(dst, src, <"[$t length]">);
  return 0;
<'    }'>
<'    fixed+struct - fixed+union - fixed+exception {'>
<'      if {[llength [$t members]]} {'>
  memcpy(dst, src, sizeof(<"[$t declarator]">));
<'      } else {'>
  (void)dst; (void)src; /* fix -Wunused-parameter */
<'      }'>
  return 0;
<'    }'>
<'    *+optional {'>
  dst->_present = src->_present;
  if (src->_present)
    return genom_tcopy_<"[[$t type] mangle]">(
      <"[[$t type] pass reference dst->_value]">,
      <"[[$t type] pass value src->_value]">);
  return 0;
<'    }'>
<'    fixed+port {'>
  memcpy(dst, src, sizeof(<"[$t declarator]">));
  return 0;
<'    }'>
<'    fixed+* {'>
  <"[$t dereference reference dst]"> = <"[$t dereference value src]">;
  return 0;
<'    }'>
<'    variable+struct - variable+exception {'>
<'      if {[llength [$t members]]} {'>
  int s;
<'        foreach e [$t members] {'>
  s = genom_tcopy_<"[$e mangle]">(
    <"[$e pass reference dst->[$e name]]">,
    <"[$e pass value src->[$e name]]">);
  if (s) return s;
<'        }'>
<'      } else {'>
  (void)dst; (void)src; /* fix -Wunused-parameter */
<'      }'>
  return 0;
<'    }'>
<'    variable+union {'>
  int s;
  switch(data->_d) {
<'      foreach e [$t members] {'>
<'        foreach v [$e value] {'>
<'          if {$v != ""} {'>
    case <"[language::cname $v]">:
<'          } else {'>
    default:
<'          }'>
<'        }'>
      s = genom_tcopy_<"[$e mangle]">(
        <"[$e pass reference dst->_u.[$e name]]">,
        <"[$e pass value src->_u.[$e name]]">);
      if (s) return s;
      break;
<'      }'>
  }
  s = genom_tcopy_<"[[$t disc] mangle]">(
    <"[[$t disc] pass reference dst->_d]">,
    <"[[$t disc] pass value src->_d]">);
  return s;
<'    }'>
<'    variable+sequence {'>
  int s;
  uint32_t i;
<'      if {[catch {$t length}]} {'>
  genom_sequence_reserve(dst, src->_maximum);
<'      }'>
  dst->_length = src->_length;
  for (i=0; i<src->_length; i++) {
    s = genom_tcopy_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {dst->_buffer[i]}]">,
      <"[[$t type] pass value {src->_buffer[i]}]">);
    if (s) return s;
  }
  return 0;
<'
    }
    variable+array {'>
  int s;
  uint32_t i;
  for (i=0; i<<"[$t length]">; i++) {
    s = genom_tcopy_<"[[$t type] mangle]">(
      <"[[$t type] pass reference {dst[i]}]">,
      <"[[$t type] pass value {src[i]}]">);
    if (s) return s;
  }
  return 0;
<'
    }
    variable+string {'>
  if (*dst) free(*dst);
  *dst = src ? strdup(src) : NULL;
  return src && !*dst ? ENOMEM : 0;
<'
      }
    default {
      error "reached unreachable"
    }
  }
'>
}

<'}'>
#endif /* H_GENOM_TYPECOPY */
