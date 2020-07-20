#
# Copyright (c) 2012-2013 LAAS/CNRS
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









proc mutex-ports-basic { dotgen codel } {
	

  set mut [list]
  foreach p [$codel parameters] {
	  if {![catch {$p port}]} {
	  
	  foreach comp [dotgen components] {
		  foreach t [$comp tasks] {
			  if {(![catch {$codel service}]) && [[$codel service] task] == $t} {continue}
			  if {(![catch {$codel task}]) && [$codel task] == $t} {continue}
			  foreach c [$t codels] {
				  foreach po [$c parameters] {
					  if {[$po name] == [$p name] && !($c in [$codel mutex])} {
						  lappend mut [join [list "lock" [$c name] [$t name] [$comp name]] _]
						  break}}
			  }
		  foreach s [$t services] {
			   foreach c [$s codels] {
				  foreach po [$c parameters] {
					  if {[$po name] == [$p name] && !($c in [$codel mutex])} {
						  lappend mut [join [list "lock" [$c name] [$s name] [$comp name]] _]
						  break}}
			  }
	  
		}}
	  }
  }} 

	return $mut
						
							
					
}






proc mutex-ports { dotgen codel } {
  set sites [list]
  foreach p [$codel parameters] {
	  if {![catch {$p port}]} {
	  set counter -1
	  set test 0
	  foreach c [dotgen components] {
		  foreach po [$c ports] {
			  if {[$po name]=="genom_state"} {continue}
			  if {[$po dir]!="in"} {
			  incr counter
			  if {[$po name] == [$p name]} {
				  lappend sites [list $counter [$p dir] [$p name] [$c name]]
				  set test 1
				  break}}}
			  if {$test} {break}}}} 

	return $sites
						
							
					
}

proc ports-names { dotgen } {
	
	set names [list]
	foreach c [dotgen components] {
		  foreach p [$c ports] { 
			  if {[$p name]=="genom_state" || [$p dir]=="in"} {continue}
			  lappend names [join [list [$p name] [$c name]] _]}}
	return $names
}

proc ports-number { dotgen } {
	set counter 0
	foreach c [dotgen components] {
		  foreach p [$c ports] { 
			  if {[$p name]=="genom_state" || [$p dir]=="in"} {continue}
			  incr counter}}
	return $counter
}

proc parameter-match { p q } {
  if {[$p base] ne [$q base]} { return no }
  set pm [$p member]
  set qm [$q member]

  if {$pm eq ""} { return yes }
  if {$qm eq ""} { return yes }
  if {[string first $pm $qm] == 0} { return yes }
  if {[string first $qm $pm] == 0} { return yes }
  return no
}

