#
# Copyright (c) 2011-2014 LAAS/CNRS
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software for any purpose
# with or without   fee is hereby granted, provided   that the above  copyright
# notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS  SOFTWARE INCLUDING ALL  IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR  BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR  ANY DAMAGES WHATSOEVER RESULTING  FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION,   ARISING OUT OF OR IN    CONNECTION WITH THE USE   OR
# PERFORMANCE OF THIS SOFTWARE.
#
#                                            Anthony Mallet on Thu Nov 24 2011
#

global rosmsg_done
set rosmsg_done [dict create]


# --- rosmsg_cname ---------------------------------------------------------
#
# Return a type's fullname converted into a ros message compatible file name
#
proc rosmsg_cname { type } {
  if {[catch {$type fullname} d]} {
    set d [$type kind]
  }

  return [string trimleft [string map {:: _} $d] _]
}


# --- rosmsg_field ---------------------------------------------------------
#
# Return ros message field type corresponding to an IDL type

proc rosmsg_field { type } {
  if {![catch {$type masquerade ros} masquerade]} {
    return $masquerade
  }

  switch -- [$type kind] {
    {boolean}			{ set m bool }
    {unsigned short}		{ set m "uint16" }
    {short}			{ set m "int16" }
    {unsigned long}		{ set m "uint32" }
    {long}			{ set m "int32" }
    {unsigned long long}	{ set m "uint64" }
    {long long}			{ set m "int64" }
    {float}			{ set m "float32" }
    {double}			{ set m "float64" }
    {char}			{ set m "int8" }
    {octet}			{ set m "uint8" }
    {string}			{ set m "string" }

    {enum}			{
      set m [rosmsg_cname $type]
    }

    {array}			{
      set m [rosmsg_field [$type type]]
      append m "\[[$type length]\]"
    }

    sequence - optional		{
      set m [rosmsg_field [$type type]]
      append m "\[\]"
    }

    typedef - struct - exception {
      set m [rosmsg_cname $type]
    }

    {struct member} -
    {union member}		{
      set m [rosmsg_field [$type type]]
    }

    default { error "[$type kind] not implemented" }
  }

  return $m
}


# --- rosmsg_* -------------------------------------------------------------
#
# Create a ros message text corresponding to an IDL type
#

proc rosmsg_enum { type } {
  set m "# IDL enum [$type fullname]\n"
  set v -1
  foreach e [$type members] {
    append m "uint32 [rosmsg_cname $e] = [incr v]\n"
  }
  append m "\n"
  append m "uint32 value\n"

  return $m
}

proc rosmsg_struct { type } {
  set m "# IDL struct [$type fullname]\n"
  foreach e [$type members] {
    append m "[rosmsg_field [$e type]] [$e name]\n"
  }

  return $m
}

proc rosmsg_exception { type } {
  set m "# IDL exception detail [$type fullname]\n"
  foreach e [$type members] {
    append m "[rosmsg_field [$e type]] [$e name]\n"
  }

  return $m
}

proc rosmsg_typedef { type } {
  set m "# IDL typedef [$type fullname]\n"
  append m "[rosmsg_field [$type type]] [$type name]\n"

  return $m
}


# --- rosmsg ---------------------------------------------------------------
#
# Generate .msg text for $type
#
proc rosmsg { type } {
  # set lang to C for mapping enum/field names into a ros msg compatible string
  lang c
  switch -- [$type kind] {
    {enum}	{ return [rosmsg_enum $type] }
    {struct}	{ return [rosmsg_struct $type] }
    {exception}	{ return [rosmsg_exception $type] }
    {typedef}	{ return [rosmsg_typedef $type] }
  }

  error "[$type kind] not implemented"
}


# --- rossrv ---------------------------------------------------------------
#
# Generate .srv/.action text for service/remote
#
proc rossrv { service } {
  set in ""
  set out "bool genom_success\nstring genom_exdetail\n"
  foreach p [$service parameters in out inout] {
    set type [$p type]
    set f "[rosmsg_field $type] [$p name]\n"
    switch -- [$p dir] {
      "in" - "inout" { append in $f }
    }
    switch -- [$p dir] {
      "out" - "inout" { append out $f }
    }
  }

  set m "# service [$service name]\n"
  append m "${in}---\n$out"
  switch [$service kind] {
    activity {
      append m "---\n"
    }
  }

  return $m
}


# --- genrosmsg ------------------------------------------------------------
#
# Generate .msg source for all $types
#
proc genrosmsg { types } {
  global rosmsg_done

  foreach type $types {
    if {[dict exists $rosmsg_done $type]} continue

    if {![catch {$type masquerade ros}]} {
      dict set rosmsg_done $type 1
      continue
    }
    switch -- [$type kind] {
      enum {}
      typedef {
        genrosmsg [$type type]
      }
      struct - union - exception {
        foreach m [$type members] { genrosmsg [$m type] }
      }

      array - sequence - optional {
        genrosmsg [$type type]
        continue
      }
      default continue
    }

    set m [rosmsg $type]
    template parse raw $m \
        file [[dotgen component] name]/msg/[rosmsg_cname $type].msg
    dict set rosmsg_done $type 1
  }

  return [dict keys $rosmsg_done]
}


# --- genrossrv ------------------------------------------------------------
#
# Generate .srv/.action source for component
#
proc genrossrv { comp {doremotes off} } {
  set pub_types [list]
  foreach p [$comp ports] {
    lappend pub_types [$p datatype]
  }
  set l [$comp services]
  if {$doremotes} { lappend l {*}[$comp remotes] }
  foreach s $l {
    set m [rossrv $s]

    foreach p [$s parameters in out inout] {
      set type [$p type]
      switch -- [[$type final] kind] {
        {array} {
          set type [[$type final] type]
        }
      }
      lappend pub_types $type
    }

    switch [$s kind] {
      attribute - function {
        template parse raw $m file [$comp name]/srv/[$s name].srv
      }
      activity {
        template parse raw $m file [$comp name]/action/[$s name].action
      }
    }
  }

  return $pub_types
}
