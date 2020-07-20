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

#ifndef H_GROS_<"$COMP">_PARAMETERS
#define H_GROS_<"$COMP">_PARAMETERS

#include <cassert>
#include <cstring>
#include <cstdlib>


/* --- Generic ------------------------------------------------------------- */

namespace genom {
  namespace ids {
    /* init */
    template<typename T> inline void pinit(T &p) { }

    template<typename T, std::size_t size> inline void pinit(T (&p)[size]) {
      for(std::size_t i = 0; i<size; i++) pinit(p[i]);
    }

    template<typename T> inline void pinit(T *(&p)) { p = 0; }

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    struct - union - sequence - optional {}
    exception { if {![llength [[$type final] members]]} continue }
    default continue
  }
'>
    template<> inline void
      pinit< <"[$type declarator]"> >(<"[$type declarator &p]">);
<'}'>


    /* fini */
    template<typename T> inline void pfini(T &p) { }

    template<typename T, std::size_t size> inline void pfini(T (&p)[size]) {
      for(std::size_t i = 0; i<size; i++) pfini(p[i]);
    }

    template<> inline void pfini<char *>(char *(&p)) {
      if (p) { free(p); p = NULL; }
    }

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    struct - union - sequence - optional {}
    exception { if {![llength [[$type final] members]]} continue }
    default continue
  }
'>
    template<> inline void
      pfini< <"[$type declarator]"> >(<"[$type declarator &p]">);
<'}'>

    template<typename T> inline void pfini(void *p) {
      genom::ids::pfini(*(T *)p);
    }


    /* copy */
    template<typename T> inline void pcopy(T &dst, const T &src) {
      dst = src;
    }

    template<typename T, std::size_t size> inline void
      pcopy(T (&dst)[size], const T (&src)[size]) {
      for(std::size_t i = 0; i<size; i++) pcopy(dst[i], src[i]);
    }

    template<> inline void pcopy<char *>(char *(&dst), char * const (&src)) {
      size_t l = src?strlen(src):0;
      if (src) dst = (char *)realloc(dst, l+1); else dst = NULL;
      if (dst) strcpy(dst, src);
    }

    inline void pcopy(char *(&dst), const char * const (&src)) {
      size_t l = src?strlen(src):0;
      if (src) dst = (char *)realloc(dst, l+1); else dst = NULL;
      if (dst) strcpy(dst, src);
    }

    inline void pcopy(char **dst, const char * const (&src)) {
      size_t l = src?strlen(src):0;
      if (src) *dst = (char *)realloc(*dst, l+1); else *dst = NULL;
      if (*dst) strcpy(*dst, src);
    }

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    struct - union - sequence - optional {}
    exception { if {![llength [[$type final] members]]} continue }
    default continue
  }
'>
    template<> inline void pcopy< <"[$type declarator]"> >(
        <"[$type declarator &dst]">, const <"[$type declarator &src]">);
<'}'>
  };
};


<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    sequence {}
    default continue
  }
'>

/* <"[--- Type [$type declarator] ------------------------------------------]"> */

namespace genom {
  namespace ids {
    template<> inline void
      pinit< <"[$type declarator]"> >(<"[$type declarator &p]">) {
<'if {[catch {$type length} l]} {'>
      p._maximum = p._length = 0;
      p._buffer = NULL;
      p._release = NULL;
<'} else {'>
      p._length = 0;
      for(std::size_t i = 0; i<<"$l">; i++) pinit(p._buffer[i]);
<'}'>
    }

    template<> inline void
      pfini< <"[$type declarator]"> >(<"[$type declarator &p]">) {
      for(std::size_t i = 0; i<p._length; i++) pfini(p._buffer[i]);
<'if {[catch {$type length}]} {'>
      if (p._release && p._buffer) p._release(p._buffer);
<'}'>
    }

    template<> inline void
      pcopy< <"[$type declarator]"> >(<"[$type declarator &dst]">,
              const <"[$type declarator &src]">) {
<'if {[catch {$type length}]} {'>
      if (genom_sequence_reserve(&dst, src._maximum))
        assert(!"out of memory");
<'}'>
      dst._length = src._length;
      for(std::size_t i = 0; i<src._length; i++)
        pcopy(dst._buffer[i], src._buffer[i]);
    }
  };
};

<'}'>

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    optional {}
    default continue
  }
'>

/* <"[--- Type [$type declarator] ------------------------------------------]"> */

namespace genom {
  namespace ids {
    template<> inline void
      pinit< <"[$type declarator]"> >(<"[$type declarator &p]">) {
      pinit(p._value);
    }

    template<> inline void
      pfini< <"[$type declarator]"> >(<"[$type declarator &p]">) {
      pfini(p._value);
    }

    template<> inline void
      pcopy< <"[$type declarator]"> >(<"[$type declarator &dst]">,
              const <"[$type declarator &src]">) {
      dst._present = src._present;
      if (src._present)
        pcopy(dst._value, src._value);
    }
  };
};

<'}'>

<'foreach type $types {
  if {[$type kind] == "typedef"} continue

  # no specialization for basic types
  switch -- [[$type final] kind] {
    struct - union {}
    exception { if {![llength [[$type final] members]]} continue }
    default continue
  }
'>

/* <"[--- Type [$type fullname] --------------------------------------------]"> */

namespace genom {
  namespace ids {
    template<> inline void
      pinit< <"[$type declarator]"> >(<"[$type declarator &p]">) {
<'  foreach m [$type members] {'>
      pinit(p.<"[$m name]">);
<'  }'>
    }

    template<> inline void
      pfini< <"[$type declarator]"> >(<"[$type declarator &p]">) {
<'  foreach m [$type members] {'>
      pfini(p.<"[$m name]">);
<'  }'>
    }

    template<> inline void
      pcopy< <"[$type declarator]"> >(<"[$type declarator &dst]">,
              const <"[$type declarator &src]">) {
<'  foreach m [$type members] {'>
      pcopy(dst.<"[$m name]">, src.<"[$m name]">);
<'  }'>
    }
  };
};

<'}'>

#endif /* H_GROS_<"$COMP">_PARAMETERS */
