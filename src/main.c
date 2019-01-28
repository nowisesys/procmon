/* procmon - runaway process monitor
 * 
 * Copyright (C) 2011-2018 Anders Lövgren, BMC-IT, Uppsala University
 * Copyright (C) 2018-2019 Anders Lövgren, Nowise Systems
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   main.c
 * Author: andlov
 *
 * Created on den 15 november 2011, 01:52
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <grp.h>
#include <pwd.h>
#include <libgen.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>

#include "procmon.h"
#include "procdisp.h"

static void usage(const char *prog, const struct proc_limit *lim)
{
	printf("%s - runaway process monitor\n", PACKAGE_NAME);
	printf("\n");
	printf("Usage: %s [options...]\n", prog);
	printf("Options:\n");
	printf("  -c,--command=name: Name of command to monitor.\n");
	printf("  -n,--limit=sec:    Max execution time limit (%lu sec).\n", lim->nsexec);
	printf("  -b,--daemon:       Fork to background running as daemon.\n");
	printf("  -x,--script=path:  Execute script when signal process.\n");
	printf("  -s,--signal=num:   Send signal to processes (%d).\n", lim->signal);
	printf("  -i,--interval=sec: Poll interval (%d sec).\n", lim->interval);
	printf("  -f,--foreground:   Don't detach from controlling terminal.\n");
	printf("  -z,--fuzzy:        Enable fuzzy match of command name.\n");
	printf("  -p,--pidfile=path: Write PID to file (%s).\n", lim->pidfile);
	printf("  -u,--user=name:    Set process user (by name).\n");
	printf("  -U,--uid=num:      Set process user (by UID).\n");
	printf("  -g,--group=name:   Set process group (by name).\n");
	printf("  -G,--gid=num:      Set process group (by GID).\n");
	printf("  -S,--secure:       Permanent drop credentials (real UID and GID).\n");
	printf("  -m,--dry-run:      Don't kill processes, only monitor and report.\n");
	printf("  -d,--debug:        Enable debug.\n");
	printf("  -v,--verbose:      Be more verbose.\n");
	printf("  -h,--help:         This help.\n");
	printf("  -V,--version:     Show version.\n");
	printf("Hint:\n");
	printf("  Use --signal=0 together with --script=path, see manual page for more info.\n");
	printf("\n");
	printf("Copyright (C) 2011-2018 Anders Lövgren, BMC-IT, Uppsala University.\n");
	printf("Copyright (C) 2018-2019 Anders Lövgren, Nowise Systems.\n");
	printf("Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

static void version(void)
{
	printf("%s\n", PACKAGE_STRING);
}

static void sigterm(int sig)
{
	if (sig == SIGKILL || sig == SIGTERM) {
		syslog(LOG_DEBUG, "Received signal %d (%s) (exiting).", sig, strsignal(sig));
		done = 1;
	}
}

static void sigint(int sig)
{
	if (sig == SIGINT) {
		fprintf(stderr, "Received signal %d (%s) (exiting).\n", sig, strsignal(sig));
		done = 1;
	}
}

static void sighup(int sig)
{
	if (sig == SIGHUP) {
		/* ignore */
	}
}

static void parse_options(int argc, char **argv, const char *prog, struct proc_limit *lim)
{
	const struct option lopts[] = {
		{ "daemon", 0, NULL, 'b'},
		{ "command", 1, NULL, 'c'},
		{ "debug", 0, NULL, 'd'},
		{ "foreground", 0, NULL, 'f'},
		{ "group", 1, NULL, 'g'},
		{ "gid", 1, NULL, 'G'},
		{ "help", 0, NULL, 'h'},
		{ "interval", 1, NULL, 'i'},
		{ "dry-run", 0, NULL, 'm'},
		{ "limit", 1, NULL, 'n'},
		{ "signal", 1, NULL, 's'},
		{ "secure", 0, NULL, 'S'},
		{ "pidfile", 1, NULL, 'p'},
		{ "user", 1, NULL, 'u'},
		{ "uid", 1, NULL, 'U'},
		{ "verbose", 0, NULL, 'v'},
		{ "version", 0, NULL, 'V'},
		{ "script", 1, NULL, 'x'},
		{ "fuzzy", 0, NULL, 'z'}
	};
	int c, index;
	opterr = 0;

	lim->prog = prog;
	lim->self = argv[0];

	lim->nsexec = PMON_DEFAULT_NSEXEC;
	lim->interval = PMON_TIMEOUT_INTERVAL;
	lim->signal = PMON_DEFAULT_SIGNAL;
	lim->pidfile = PMON_DEFAULT_PIDFILE;
	lim->ticks = sysconf(_SC_CLK_TCK);

	lim->euid = lim->ruid = getuid();
	lim->egid = lim->rgid = getgid();

	while ((c = getopt_long(argc, argv, "bc:dfg:G:hi:mn:p:s:Su:U:vVx:z", lopts, &index)) != -1) {
		switch (c) {
		case 'b':
			lim->daemon = 1;
			break;
		case 'c':
			lim->exename = optarg;
			break;
		case 'd':
			lim->debug++;
			break;
		case 'f':
			lim->fgmode = 1;
			break;
		case 'g':
		{
			struct group *gr;

			if ((gr = getgrnam(optarg))) {
				lim->egid = gr->gr_gid;
			} else {
				perror("getgrnam");
				exit(1);
			}
		}
			break;
		case 'G':
			lim->egid = atoi(optarg);
			break;
		case 'h':
			usage(prog, lim);
			exit(0);
		case 'i':
			lim->interval = atoi(optarg);
			break;
		case 'm':
			lim->dryrun = 1;
			break;
		case 'n':
			lim->nsexec = atoi(optarg);
			break;
		case 'p':
			lim->pidfile = optarg;
			break;
		case 's':
			lim->signal = atoi(optarg);
			break;
		case 'S':
			lim->secure = 1;
			break;
		case 'u':
		{
			struct passwd *pw;

			if ((pw = getpwnam(optarg))) {
				lim->euid = pw->pw_uid;
			} else {
				perror("getpwnam");
				exit(1);
			}
		}
			break;
		case 'U':
			lim->euid = atoi(optarg);
			break;
		case 'v':
			lim->verbose++;
			break;
		case 'V':
			version();
			exit(0);
		case 'x':
			lim->script = optarg;
			break;
		case 'z':
			lim->fuzzy = 1;
			break;
		case '?':
		default:
			fprintf(stderr, "%s: invalid option '%s', see --help\n", prog, argv[optind - 1]);
			exit(1);
		}
	}

	if (lim->exename) {
		lim->cmdline = strchr(lim->exename, '/') != NULL;
	}
	if (lim->fuzzy) {
		lim->cmdline = 1;
	}
	if (strcmp(lim->prog, "procmond") == 0) {
		lim->daemon = 1;
	}
}

static void pmon_run(struct proc_limit *lim)
{
	int res = 0, fd;

	if (lim->daemon) {
		if (!lim->fgmode) {
			if (daemon(0, 0) < 0) {
				perror("daemon");
				exit(1);
			}
		} else {
			printf("Running in interactive mode (undetached). Press Ctrl+C to exit.\n");
		}
		openlog(lim->prog, LOG_PID, LOG_DAEMON);

		snprintf(lim->pidbuff, sizeof(lim->pidbuff), "%d\n", getpid());
		if ((fd = open(lim->pidfile, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
			error("Failed open %s (%s)", lim->pidfile, strerror(errno));
			exit(1);
		}
		if (write(fd, lim->pidbuff, strlen(lim->pidbuff)) < 0) {
			error("Failed write %s (%s)", lim->pidfile, strerror(errno));
			exit(1);
		} else {
			close(fd);
		}

#ifdef HAVE_CHOWN
		if (lim->euid != lim->ruid || lim->egid != lim->rgid) {
			if (chown(lim->pidfile, lim->euid, lim->egid) < 0) {
				error("Failed chown uid=%d, gid=%d on %s (%s)",
					lim->euid, lim->egid, lim->pidfile, strerror(errno));
				exit(1);
			}
		}
#endif

		sigfillset(&lim->sigset);
		sigdelset(&lim->sigset, SIGKILL);
		sigdelset(&lim->sigset, SIGTERM);
		sigdelset(&lim->sigset, SIGINT);
		sigdelset(&lim->sigset, SIGHUP);
		sigprocmask(SIG_SETMASK, &lim->sigset, NULL);
		signal(SIGKILL, sigterm);
		signal(SIGTERM, sigterm);
		signal(SIGINT, sigint);
		signal(SIGHUP, sighup);

		if (pmon_secure(lim, PMON_SECURE_INIT) < 0) {
			exit(1);
		}
		if (!lim->fgmode) {
			info("Daemon starting up... (%s)", PACKAGE_STRING);
		}

		while (!done) {
			struct timeval tv;
			tv.tv_sec = lim->interval;
			tv.tv_usec = 0;
			if (select(0, NULL, NULL, NULL, &tv) < 0) {
				if (!done) { /* watchout for interupted syscall */
					error("Failed call select: %s", strerror(errno));
					continue;
				} else {
					break;
				}
			}
			if ((res = pmon_scan(lim)) < 0) {
				error("Error in process scanner");
				done = 1;
			}
		}

		if (!lim->fgmode) {
			info("Daemon exiting (%s)", PACKAGE_STRING);
		}
		if (pmon_secure(lim, PMON_SECURE_DONE) < 0) {
			exit(1);
		}
		if (unlink(lim->pidfile) < 0) {
			warn("Failed delete %s (%s)", lim->pidfile, strerror(errno));
		}
		closelog();
	} else {
		if (pmon_scan(lim) < 0) {
			exit(1);
		}
	}

	if (res < 0) {
		exit(1);
	}
}

int main(int argc, char** argv)
{
	struct proc_limit lim;
	const char *prog = basename(argv[0]);

	memset(&lim, 0, sizeof(struct proc_limit));
	parse_options(argc, argv, prog, &lim);

	pmon_dump(&lim);
	pmon_run(&lim);

	return 0;
}

