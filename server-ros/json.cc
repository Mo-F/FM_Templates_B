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
#                                      Anthony Mallet on Wed Mar 27 2013
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]

# remote exception list
set rexl [list]
foreach r [$component remotes] {
  foreach t [$r throws] {
    if {[lsearch -exact $rexl $t] < 0} { lappend rexl $t }
  }
}


lang c
'>
#include "autoconf/acheader.h"

#include <string>

#include "<"$comp">_c_types.h"
#include "<"$comp">_json-types.h"
#include "<"$comp">_typecopy.h"

<'foreach t [$component throw] {'>

/* <"[--- Exception [$t fullname] encoding ------------------------------]"> */

void
genom_<"$comp">_<"[$t mangle]">_encodex(
  std::string &json, const <"[$t argument reference exdetail]">)
{
  json = "{\"ex\":\"<"[$t fullname]">\"";
<'  if {[llength [$t members]]} {'>
  char *jdetail, *end;
  size_t len;

  len = JSON_MINBUF;
  jdetail = end = (char *)malloc(len);
  if (jdetail) {
    if (!json_print_<"[$t mangle]">(&jdetail, &end, &len, exdetail)) {
      json += ",\"detail\":";
      json += jdetail;
    }
    if (jdetail) free(jdetail);
  }
<'  }'>
  json += "}";
}

<'}'>


/* --- Exception genom::unkex encoding/decoding ---------------------------- */

void
genom_<"$comp">_genom_unkex_encodex(
  std::string &json, const genom_unkex_detail *exdetail)
{
  json = "{\"ex\":\"::genom::unkex\",detail:{\"what\":\"";
  json += exdetail->what;
  json += "\"}}";
}

static int
json_scan_genom_unkex(genom_unkex_detail *exdetail, const char **json)
{
  const char *e;

  json_skip_whitespace(*json);
  if (*((*json)++) != '{') return EINVAL;
  json_skip_whitespace(*json);
  while(**json != '}') {
    if (*((*json)++) != '"') return EINVAL;

    if (!strncmp(*json, "what\"", sizeof("what"))) {
      (*json) += sizeof("what");
      json_skip_whitespace(*json);
      if (*((*json)++) != ':') return EINVAL;
      json_skip_whitespace(*json);
      if (**json != '"') return EINVAL;
      e = ++(*json);
      while (**json != '"') { if (**json == '\\') (*json)++; (*json)++; }
      if ((size_t)(*json - e) >= sizeof(exdetail->what))
        e = *json - sizeof(exdetail->what);
      strncpy(exdetail->what, e, *json - e);
      exdetail->what[*json - e] = *"";

    } else return EINVAL;

    json_skip_whitespace(*json);
    if (**json == ',') {
      (*json)++;
      json_skip_whitespace(*json);
    }
  }

  (*json)++;
  return 0;
}


/* --- Exception genom::syserr encoding/decoding --------------------------- */

void
genom_<"$comp">_genom_syserr_encodex(
  std::string &json, const genom_syserr_detail *exdetail)
{
  char buf[16];

  json = "{\"ex\":\"::genom::syserr\",detail:{\"code\":";
  snprintf(buf, sizeof(buf), "%d", exdetail->code);
  json += buf;
  json += "}}";
}

static int
json_scan_genom_syserr(genom_syserr_detail *exdetail, const char **json)
{
  long long v;
  char *d;

  json_skip_whitespace(*json);
  if (*((*json)++) != '{') return EINVAL;
  json_skip_whitespace(*json);
  while(**json != '}') {
    if (*((*json)++) != '"') return EINVAL;

    if (!strncmp(*json, "code\"", sizeof("code"))) {
      (*json) += sizeof("code");
      json_skip_whitespace(*json);
      if (*((*json)++) != ':') return EINVAL;
      json_skip_whitespace(*json);

      v = strtoll(*json, &d, 0);
      if (d == *json) return EINVAL;
      *json = d;
      exdetail->code = (uint32_t)v;

    } else return EINVAL;

    json_skip_whitespace(*json);
    if (**json == ',') {
      (*json)++;
      json_skip_whitespace(*json);
    }
  }

  (*json)++;
  return 0;
}


/* --- Generic exception encoding/decoding --------------------------------- */

void
genom_<"$comp">_encodex(std::string &json, genom_event ex, const void *exdetail)
{
<'foreach t [$component throw] {'>
  if (ex == <"[$t cname]">_id) {
    genom_<"$comp">_<"[$t mangle]">_encodex(
      json, (const <"[$t argument reference]">)exdetail);
    return;
  }
<'}'>

  genom_unkex_detail d;
  strncpy(d.what, ex, sizeof(d.what)); d.what[sizeof(d.what)-1] = *"";
  genom_<"$comp">_genom_unkex_encodex(json, &d);
}

genom_event
genom_<"$comp">_decodex(const char *json, genom_context self)
{
  genom_unkex_detail unkex;
  genom_event ex = genom_ok;
  const char *e;
  int s;

  json_skip_whitespace(json);
  if (*(json++) != '{') { s = EINVAL; goto done; }
  json_skip_whitespace(json);
  while(*json != '}') {
    if (*(json++) != '"') { s = EINVAL; goto done; };

    /* look for exception name */
    if (!strncmp(json, "ex\"", sizeof("ex"))) {
      json += sizeof("ex");
      json_skip_whitespace(json);
      if (*(json++) != ':') { s = EINVAL; goto done; }
      json_skip_whitespace(json);
      if (*json != '"') { s = EINVAL; goto done; }
      e = ++json;
      while (*json != '"') { if (*json == '\\') json++; json++; }
<'foreach t $rexl {'>
      if (sizeof(<"[$t cname]">_id) == json - e + 1 &&
          !strncmp(<"[$t cname]">_id, e, sizeof(<"[$t cname]">_id)-1))
        ex = <"[$t cname]">_id;
      else
<'}'>
      {
        if ((size_t)(json - e) >= sizeof(unkex.what))
          e = json - sizeof(unkex.what);
        strncpy(unkex.what, e, json-e); unkex.what[sizeof(unkex.what)-1] = *"";
        return genom_unkex(&unkex, self);
      }
      json++;

    /* look for exception detail */
    } else if (!strncmp(json, "detail\"", sizeof("detail"))) {
      json += sizeof("detail");
      json_skip_whitespace(json);
      if (*(json++) != ':') { s = EINVAL; goto done; }

<'foreach t $rexl {'>
      if (ex == <"[$t cname]">_id) {
<'  if {[llength [$t members]]} {'>
        <"[$t declarator detail]">;
        genom::ids::pinit(detail);
        s = json_scan_<"[$t mangle]">(&detail, &json);
        if (s) goto done;
        return <"[$t cname]">(&detail, self);
<'  } else {'>
        s = EINVAL; goto done;
<'  }'>
      }
<'}'>
      if (ex == genom_syserr_id) {
        genom_syserr_detail detail;
        s = json_scan_genom_syserr(&detail, &json);
        if (s) goto done;
        return genom_syserr(&detail, self);
      }
      if (ex == genom_unkex_id) {
        genom_unkex_detail detail;
        s = json_scan_genom_unkex(&detail, &json);
        if (s) goto done;
        return genom_unkex(&detail, self);
      }

    } else { s = EINVAL; goto done; }

    json_skip_whitespace(json);
    if (*json == ',') {
      json++;
      json_skip_whitespace(json);
    }
  }

  if (ex == genom_ok) {
    strcpy(unkex.what, "::genom::ok");
    return genom_unkex(&unkex, self);
  }
  return self->raise(ex, NULL, 0, self);

done:
  {
    genom_syserr_detail d = { s };
    return genom_syserr(&d, self);
  }
}
