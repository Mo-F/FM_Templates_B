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
#                                      Anthony Mallet on Wed Feb 29 2012
#

if {[llength $argv] != 1} { error "expected arguments: component" }
lassign $argv component

# compute handy shortcuts
set comp [$component name]
set COMP [string toupper [$component name]]
if {[catch { $component version } version]} { set version {}}

lang c
'>
#include "autoconf/acheader.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "portLib.h"
#include "h2devLib.h"
#include "h2initGlob.h"

#include "<"$comp">_control_task.h"


/* --- global data --------------------------------------------------------- */

struct genom_component_data *<"$comp">_genom_component = NULL;
const char *genom_instance = "<"$comp">";
size_t <"$comp">_varmsg_maxsize = 32768;


/* --- local data ---------------------------------------------------------- */

/* options */
static const char usage_string[] = "\
  -f,--force        bypass multiple component instances detection\n\
     --maxmsg=size  set maximum message size in kB\n\
     --version      display <"$comp"> version number\n\
  -d,--debug        log extra debugging information on stderr\n\
  -h,--help         print usage summary (this text)\n\
";
static const char shortopts_string[] = "+bfi:dh";
static struct option longopts_list[] = {
  { "force",		no_argument,		NULL,	'f'},
  { "maxmsg",		required_argument,	NULL,	-'m'},
  { "version",		no_argument,		NULL,	-'v'},
  { "debug",		no_argument,		NULL,	'd'},
  { "help",		no_argument,		NULL,	'h'},
  { NULL, 0, NULL, 0}
};

static int force = 0;
static int debug = 0;

struct genom_component_data *
	BIP_genom_<"$comp">_init(void);
void BIP_genom_<"$comp">_finish(struct genom_component_data *<"$comp">_self);

static void	genom_<"$comp">_vacuum(const char *pidfile);
static void	usage(FILE *channel, char *argv0);


/* --- main ---------------------------------------------------------------- */

/** Initialization and startup
 */
int
BIP_main_init(int argc, char **argv)	/* This is the entry point used by BIP to initialize the module. */
{
  char *e;
  int c;

  /* parse command line options */
  while (
    (c = getopt_long(argc, argv, shortopts_string, longopts_list, NULL)) != -1)
    switch (c) {
      case 0: break;

      case 'f': force++; break;
      case 'd': debug++; break;

      case -'m': {
        unsigned long s = strtoul(optarg, &e, 0);
        if (*e != 0 || s <= 0) {
          genom_log_warn(0, "bad message size: %s", optarg);
          exit(1);
        }
        <"$comp">_varmsg_maxsize = s * 1024;
        break;
      }

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
  if (argc > 0) {
    usage(stderr, argv[0]);
    exit(1);
  }

  return 0;
}

char* dummy_args[] = {"<"$comp">", "-f", "-d", NULL };

void BIP_<"$comp">_init_genom(void) 
{
  char **argv = dummy_args;
  int argc = sizeof(dummy_args)/sizeof(dummy_args[0]) - 1;
  
  genom_log_debug("Initializing GenoM component from BIP.");
  BIP_main_init(argc,argv);
  <"$comp">_genom_component = BIP_genom_<"$comp">_init();	/* This will call genom_<"$comp">_init() */
}

int BIP_<"$comp">_init_mbox(void) 
{
  if (! <"$comp">_genom_component) return 0;

  genom_log_debug("Initializing MBOX from BIP.");
  <"$comp">_cntrl_task_init(<"$comp">_genom_component); /* This is a BIPE call */
  return 1;
}

/* <"[--- BIP_genom_${comp}_init ---------------------------------------------------]"> */

/* Make it static for this component? will break if more than one instance of the same module */
static char pid_file_path[PATH_MAX];

struct genom_component_data *
BIP_genom_<"$comp">_init(void)
{
  struct utsname uts;
  void *<"$comp">_self;
  int pid_file_fd;
  FILE *pid_file;
  char *pid_dir;
  sigset_t sset;
  sig_t handler;
  STATUS s;
  int sig;

  genom_log_info("Calling h2initGlob.");
  s = h2initGlob(0);		/* 0 to avoid timers (timers are handled by BIP anyway). */
  if (s == ERROR) {
    genom_log_warn(1, "h2initGlob error");
    exit(2);
  }

  /* create pid file */
  pid_dir = getenv("H2DEV_DIR");
  if (!pid_dir) pid_dir = getenv("HOME");
  if (!pid_dir) pid_dir = "/tmp";
  assert(pid_dir);

  if (uname(&uts)) {
    genom_log_warn(-1, "uname");
    exit(2);
  }

  snprintf(pid_file_path, PATH_MAX, "%s/.%s.pid-%s",
           pid_dir, genom_instance, uts.nodename);
  pid_file_fd = open(pid_file_path, O_CREAT|O_EXCL|O_WRONLY, 0644);
  if (pid_file_fd < 0) {
    if (force && errno == EEXIST) {
      genom_log_warn(-1, "error creating %s", pid_file_path);
      genom_log_info("forcibly disabling the protection against duplicate"
                     " instances");
      genom_<"$comp">_vacuum(pid_file_path);
      pid_file_fd = open(pid_file_path, O_CREAT|O_EXCL|O_WRONLY, 0644);
    }
    if (pid_file_fd < 0) {
      genom_log_warn(-1, "error creating %s", pid_file_path);
      exit(2);
    }
  }

  pid_file = fdopen(pid_file_fd, "w");
  if (!pid_file) {
    genom_log_warn(-1, "error opening %s", pid_file_path);
    exit(2);
  }
  fprintf(pid_file, "%d\n", getpid());
  fclose(pid_file);

  /* initialize component */
  <"$comp">_self = genom_<"$comp">_init();
  if (!<"$comp">_self) {
    genom_log_warn(0, "initialization failed");
    unlink(pid_file_path);
    exit(2);
  }
  
  char tname[64];

  /* make the thread a pocolibs task. This is required to use mailboxes from a thread. */
  snprintf(tname, sizeof(tname), "%s", genom_instance);
  genom_log_info("Calling taskFromThread from BIP_genom_<"$comp">_init (main).");
  taskFromThread(tname);


  return <"$comp">_self;
}


void
BIP_genom_<"$comp">_finish(struct genom_component_data *<"$comp">_self)
{
  genom_log_info("terminating on BIP request");
  genom_<"$comp">_fini(<"$comp">_self);
  unlink(pid_file_path);
}


/* <"[--- genom_${comp}_vacuum ------------------------------------------]"> */

static void
genom_<"$comp">_vacuum(const char *pidfile)
{
  char n[H2_DEV_MAX_NAME];

  if (unlink(pidfile))
    genom_log_warn(-1, "error unlinking %s", pidfile);

  snprintf(n, sizeof(n), "%s/*", genom_instance);
  genom_log_info("cleaning pocolibs %s device", genom_instance);
  h2devClean(genom_instance);
  genom_log_info("cleaning pocolibs %s devices", n);
  h2devClean(n);
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
	  "<"[string trim "$comp $version"]"> GenoM component\n\nUsage:\n"
	  "  %1$s [-i name] [--maxmsg size] [-f] [-b] [-d]\n"
	  "  %1$s [-h|--version]\n"
	  "\n%2$s",
	  basename(argv0), usage_string);
}


/* --- log ----------------------------------------------------------------- */

void
genom_log_info(const char *format, ...)
{
  va_list va;

  printf("\t[GenoM3] %s ", genom_instance);
  va_start(va, format);
  vprintf(format, va);
  va_end(va);
  printf("\n");
}

void
genom_log_warn(int h2error, const char *format, ...)
{
  va_list va;

  fprintf(stderr, "\t[GenoM3] %s ", genom_instance);
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  if (h2error) {
    fprintf(stderr, ": ");
    if (h2error > 0 ) h2perror(NULL); else perror(NULL);
  } else fprintf(stderr, "\n");
}

void
genom_log_debug(const char *format, ...)
{
  va_list va;
  if (!debug) return;

  fprintf(stderr, "\t[GenoM3] %s ", genom_instance);
  va_start(va, format);
  vfprintf(stderr, format, va);
  va_end(va);
  fprintf(stderr, "\n");
}
