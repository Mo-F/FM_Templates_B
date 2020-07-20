/*
 * Copyright (c) 2012-2013 LAAS/CNRS
 * All rights reserved.
 *
 * Redistribution and use  in source  and binary  forms,  with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   1. Redistributions of  source  code must retain the  above copyright
 *      notice and this list of conditions.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice and  this list of  conditions in the  documentation and/or
 *      other materials provided with the distribution.
 *
 *					Anthony Mallet on Fri Feb  3 2012
 */

#ifndef H_GENOM_SERIALIZE
#define H_GENOM_SERIALIZE

#ifdef __linux__
/* Apparently we need _GNU_SOURCE defined to get access to strnlen on Linux */
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
#endif

#include <err.h>
#include <stdint.h>

#include <cstdlib>
#include <cstring>

#include "ros/serialization.h"


/* --- extend ros serializers to basic types ------------------------------- */

namespace ros {
  namespace serialization {
    /* fixed-length strings actually map to (variable size) strings in ros msg,
     * due to the lack of string[N] support. */
    template<size_t N> struct Serializer<char [N]> {
      template<typename Stream> inline static void
        write(Stream& stream, const char (&a)[N]) {
        const size_t l = strnlen(a, N-1);
        stream.next((uint32_t)l);
        if (l > 0)
          memcpy(stream.advance(l), a, l);
      }

      template<typename Stream> inline static void
        read(Stream& stream, char (&a)[N]) {
        uint32_t l;
        char *b;
        stream.next(l);
        b = (char *)stream.advance(l);
        if (l >= N) l = N-1;
        memcpy(a, b, l);
        a[l] = '\0';
      }

      inline static uint32_t
        serializedLength(const char (&a)[N]) {
        return sizeof(uint32_t) + strnlen(a, N-1);
      }
    };

    template<> struct Serializer<char *> {
      template<typename Stream> inline static void
        write(Stream& stream, const char *a) {
        const size_t l = a ? strlen(a) : 0;
        stream.next((uint32_t)l);
        if (l > 0)
          memcpy(stream.advance(l), a, l);
      }

      template<typename Stream> inline static void
        read(Stream& stream, char *(&a)) {
        uint32_t l;
        char *b;
        stream.next(l);

        b = (char *)realloc(a, l+1);
        if (!b) return;
        b[l] = 0;
        a = b;
        if (l == 0) return;
        memcpy(a, stream.advance(l), l);
      }

      inline static uint32_t
        serializedLength(const char *a) {
        return sizeof(uint32_t) + (a ? strlen(a) : 0);
      }
    };
  };
};


#endif /* H_GENOM_SERIALIZE */
