<'
# Copyright (c) 2011-2015 LAAS/CNRS
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
#                                      Anthony Mallet on Fri Dec  9 2011
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
if {[catch { $component version } version]} { set version {}}

lang c
'>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdio>
#include <getopt.h>
#include <libgen.h>
#include <signal.h>
#include <stdlib.h>

#include "ros/ros.h"

#include "<"$comp">_internals.h"

/* options */
static const char usage_string[] = "\
  -b,--background   daemonize\n\
  -i,--name=string  set instance name\n\
     --version      display the version number of the invoked GenoM\n\
  -d,--debug        log extra debugging information on stderr\n\
  -h,--help         print usage summary (this text)\n\
";
static const char shortopts_string[] = "+bi:dh";
static struct option longopts_list[] = {
  { "background",	no_argument,		NULL,	'b'},
  { "name",		required_argument,	NULL,	'i'},
  { "version",		no_argument,		NULL,	-'v'},
  { "debug",		no_argument,		NULL,	'd'},
  { "help",		no_argument,		NULL,	'h'},
  { NULL, 0, NULL, 0}
};

static int debug = 0;
int genom_shutdown = 0;
const char *genom_instance = "<"$comp">";

static void	apocalypse(int sig);
static void	usage(FILE *channel, char *argv0);


/* --- main ---------------------------------------------------------------- */

/** Initialization and startup
 */
int
main(int argc, char **argv)
{
  int s, c;
  int daemonize = 0;

  /* parse command line options */
  while (
    (c = getopt_long(argc, argv, shortopts_string, longopts_list, NULL)) != -1)
    switch (c) {
      case 0: break;

      case 'b': daemonize++; break;
      case 'd': debug++; break;
      case 'i': genom_instance = optarg; break;

      case -'v': puts("<"$version">"); exit(0); break;
      case 'h':  usage(stdout, argv[0]);  exit(0); break;

      case '?':
      default:
	usage(stderr, argv[0]);
        exit(1);
        break;
    }
  argc -= optind;
  argv += optind;

  /* daemonize */
  if (daemonize) {
    sigset_t sset, oset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sset, &oset);

    switch(fork()) {
      case -1:
        perror("cannot fork");
        exit(2);

      case 0: /* child */ {
        int fd;

        sigprocmask(SIG_SETMASK, &oset, NULL);

        if (setsid() == -1) exit(-1);
        if (chdir("/")) /*noop*/;

        fd = open("/dev/null", O_RDWR, 0);
        if (fd != -1) {
          (void)dup2(fd, STDIN_FILENO);
          if (fd > STDERR_FILENO)
            (void)close(fd);
        }
        signal(SIGUSR1, SIG_IGN);
        break;
      }

      default: /* parent */ {
        int sig = 0;

        do { sigwait(&sset, &sig); } while (sig == 0);
        assert(sig == SIGUSR1);
        _exit(0);
      }
    }
  }

  signal(SIGINT, apocalypse);
  signal(SIGQUIT, apocalypse);
  signal(SIGTERM, apocalypse);

  /* block SIGALRM if the component needs a timer
   * block SIGUSR{1, 2} and SIGCHLD too for codel purpose
   * XXX until a better solution is found
   */
  sigset_t sset;
  sigemptyset(&sset);
<'if {![catch {$component clock-rate} rate]} {'>
  sigaddset(&sset, SIGALRM);
<'}'>
  sigaddset(&sset, SIGUSR1);
  sigaddset(&sset, SIGUSR2);
  sigaddset(&sset, SIGCHLD);

  pthread_sigmask(SIG_BLOCK, &sset, NULL);

  try {
    ros::init(argc, argv, genom_instance, ros::init_options::NoSigintHandler);
  } catch (std::exception &e) {
    genom_log_warn("%s", e.what());
    exit(2);
  }
  s = <"$comp">_cntrl_task(genom_instance);

  return s;
}


/* --- signal handler ------------------------------------------------------ */

static void
apocalypse(int sig)
{
  genom_shutdown = 1;
}


/* --- usage --------------------------------------------------------------- */

/** Print usage on a channel.
 *
 * \param[in] channel	Output channel
 * \param[in] argv0	Program invocation name
 */

static void
usage(FILE *channel, char *argv0)
{
  fprintf(channel,
	  "<"[string trim "$comp $version"]"> GenoM component\n\n"
	  "  %1$s [-i name] [-b] [-d]\n"
	  "  %1$s [-h|--version]\n"
	  "\n%2$s",
	  basename(argv0), usage_string);
}


/* --- log ----------------------------------------------------------------- */

void
genom_log_info(const char *format, ...)
{
  va_list va;

  printf("%s: ", genom_instance);
  va_start(va, format);
  vprintf(format, va);
  va_end(va);
  printf("\n");
}

void
genom_log_warn(const char *format, ...)
{
  va_list va;

  fprintf(stderr, "%s: ", genom_instance);
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  fprintf(stderr, "\n");
}

void
genom_log_debug(const char *format, ...)
{
  struct timeval tv;
  struct tm tm;
  va_list va;
  if (!debug) return;

  gettimeofday(&tv, NULL);
  localtime_r(&tv.tv_sec, &tm);
  fprintf(stderr, "%02d:%02d:%02d.%06d %s: ",
          tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)tv.tv_usec,
          genom_instance);
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  fprintf(stderr, "\n");
}
