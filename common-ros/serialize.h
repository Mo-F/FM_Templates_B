<'
# Copyright (c) 2011-2014 LAAS/CNRS
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
#                                      Anthony Mallet on Tue Nov 29 2011
#
#

# check arguments
if {[llength $argv] != 1} { error "expected arguments: types" }
lassign $argv types

lang c

# compute handy shortcuts
set component [dotgen component]
set comp [$component name]
set COMP [string toupper [$component name]]
'>

#ifndef H_GROS_<"$COMP">_SERIALIZE
#define H_GROS_<"$COMP">_SERIALIZE

#include <cassert>

#include "ros/serialization.h"

#include "genom-serialize.h"


/* === Serialization methods =============================================== */

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no serialization for basic types
  switch -- [[$type final] kind] {
    {boolean}				-
    {unsigned short} - {short}		-
    {unsigned long} - {long}		-
    {unsigned long long} - {long long}	-
    {float} - {double}			-
    {char} - {octet}			-
    {enum} - {string}			continue

    sequence - optional - array {
      set stype [[$type type] final]
      switch -- [$stype kind] {
        {boolean}				-
        {unsigned short} - {short}		-
        {unsigned long} - {long}		-
        {unsigned long long} - {long long}	-
        {float} - {double}			-
        {char} - {octet}			-
        {enum} { set sbasic 1 }
        default { set sbasic 0 }
      }
    }

    exception { if {![llength [[$type final] members]]} continue }
  }
'>

<'
  if {[catch {$type fullname} fullname]} {
    set fullname [$type declarator]
  }
'>
/* <"[--- Type $fullname ------------------------------------------------]"> */

namespace ros {
  namespace serialization {
    template<> struct Serializer< <"[$type declarator]"> > {

      template<typename Stream> inline static void
        write(Stream& stream, const <"[$type declarator (&a)]">) {
<'  switch -- [$type kind] {'>
<'    array {'>
<'      if {$sbasic} {'>
        memcpy(stream.advance(sizeof(a)), a, sizeof(a));
<'        } else {'>
        for(uint32_t i = 0; i < <"[$type length]">; i++) {
          stream.next(a[i]);
        }
<'        }'>
<'    }'>
<'    sequence {'>
        stream.next(a._length);
<'        if {$sbasic} {'>
        memcpy(stream.advance(a._length * sizeof(*a._buffer)),
               a._buffer, a._length * sizeof(*a._buffer));
<'        } else {'>
        for(uint32_t i = 0; i < a._length; i++) {
          stream.next(a._buffer[i]);
        }
<'        }'>
<'    }'>
<'    optional {'>
        if (a._present) {
          stream.next((uint32_t)1);
<'        if {$sbasic} {'>
          memcpy(stream.advance(sizeof(a._value)), a._value, sizeof(a._value));
<'        } else {'>
          stream.next(a._value);
<'        }'>
        } else {
          stream.next((uint32_t)0);
        }
<'    }'>
<'    struct - union - exception {'>

<'      foreach m [$type members] {'>
        stream.next(a.<"[$m name]">);
<'      }'>

<'    }'>
<'    default {'>
        stream.next(a);
<'    }'>
<'  }'>
      }

      template<typename Stream> inline static void
        read(Stream& stream, <"[$type declarator (&a)]">) {
<'  switch -- [$type kind] {'>
<'    array {'>
<'      if {$sbasic} {'>
        memcpy(a, stream.advance(sizeof(a)), sizeof(a));
<'        } else {'>
        for(uint32_t i = 0; i < <"[$type length]">; i++) {
          stream.next(a[i]);
        }
<'        }'>
<'    }'>
<'    sequence {'>
        uint32_t l;
        stream.next(l);
<'      if {[catch {$type length} l]} {'>
        if (genom_sequence_reserve(&a, l)) assert(!"out of memory");
<'      }'>
        a._length = l;
<'      if {![catch {$type length} l]} {'>
        if (a._length > <"$l">) a._length = <"$l">;
<'      }'>
<'      if {$sbasic} {'>
        memcpy(a._buffer, stream.advance(a._length * sizeof(*a._buffer)),
               a._length * sizeof(*a._buffer));
<'      } else {'>
        uint32_t i;
        <"[[$type type] declarator x]">;
        for(i = 0; i < a._length; i++) stream.next(a._buffer[i]);
        for(; i < l; i++) stream.next(x);
<'      }'>
<'    }'>
<'    optional {'>
        uint32_t p;
        stream.next(p);
        a._present = p?1:0;
        if (p) {
<'      if {$sbasic} {'>
          memcpy(&a._value, stream.advance(sizeof(a._value)), sizeof(a._value));
<'      } else {'>
          stream.next(a._value);
<'      }'>
          /* discard extraneous elements (buggy client) */
          <"[[$type type] declarator x]">;
          for(uint32_t i = 1; i < p; i++) stream.next(x);
        }
<'    }'>
<'    struct - union - exception {'>

<'      foreach m [$type members] {'>
        stream.next(a.<"[$m name]">);
<'      }'>

<'    }'>
<'    default {'>
        stream.next(a);
<'    }'>
<'  }'>
      }

      inline static uint32_t
        serializedLength(const <"[$type declarator (&a)]">) {
<'  switch -- [$type kind] {'>
<'    array {'>
<'      if {$sbasic} {'>
        return sizeof(a);
<'        } else {'>
        return <"[$type length]"> * serializationLength(a[0]);
<'        }'>
<'    }'>
<'    sequence {'>
<'      if {$sbasic} {'>
        return 4 + a._length * sizeof(*a._buffer);
<'      } else {'>
        uint32_t s = 4;
        for(uint32_t i = 0; i < a._length; i++) {
          s += serializationLength(a._buffer[i]);
        }
        return s;
<'      }'>
<'    }'>
<'    optional {'>
<'      if {$sbasic} {'>
        return 4 + (a._present ? sizeof(a._value) : 0);
<'      } else {'>
        return 4 + (a._present ? serializationLength(a._value) : 0);
<'      }'>
<'    }'>
<'    struct - union - exception {'>

        uint32_t s = 0;
<'      foreach m [$type members] {'>
        s += serializationLength(a.<"[$m name]">);
<'      }'>
        return s;

<'    }'>
<'    default {'>
        return serializationLength(a);
<'    }'>
<'  }'>
      }
    };
  };
};

<'}'>

#endif /* H_GROS_<"$COMP">_SERIALIZE */
