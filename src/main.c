/* procmon - runaway process monitor
 * Copyright (C) 2011 Anders Lövgren, Compdept at BMC, Uppsala University
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
	printf("  -m,--dry-run:      Don't kill processes, only monitor and report.\n");
	printf("  -d,--debug:        Enable debug.\n");
	printf("  -v,--verbose:      Be more verbose.\n");
	printf("  -h,--help:         This help.\n");
	printf("  -V,--version:     Show version.\n");
	printf("Hint:\n");
	printf("  Use --signal=0 together with --script=path, see manual page for more info.\n");
	printf("\n");
	printf("Copyright (C) 2011 Anders Lövgren, Compdept at BMC, Uppsala University.\n");
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
		{ "help", 0, NULL, 'h'},
		{ "interval", 1, NULL, 'i'},
		{ "dry-run", 0, NULL, 'm'},
		{ "limit", 1, NULL, 'n'},
		{ "signal", 1, NULL, 's'},
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
	lim->ticks = sysconf(_SC_CLK_TCK);

	while ((c = getopt_long(argc, argv, "bc:dfhi:mn:s:vVx:z", lopts, &index)) != -1) {
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
		case 's':
			lim->signal = atoi(optarg);
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

int main(int argc, char** argv)
{
	int res = 0;
	struct proc_limit lim;
	const char *prog = basename(argv[0]);

	memset(&lim, 0, sizeof(struct proc_limit));
	parse_options(argc, argv, prog, &lim);

	pmon_dump(&lim);

	if (lim.daemon) {
		if (!lim.fgmode) {
			if (daemon(0, 0) < 0) {
				perror("daemon");
				exit(1);
			}
		} else {
			printf("Running in interactive mode (undetached). Press Ctrl+C to exit.\n");
		}
		openlog(prog, LOG_PID, LOG_DAEMON);
		sigfillset(&lim.sigset);
		sigdelset(&lim.sigset, SIGKILL);
		sigdelset(&lim.sigset, SIGTERM);
		sigdelset(&lim.sigset, SIGINT);
		sigdelset(&lim.sigset, SIGHUP);
		sigprocmask(SIG_SETMASK, &lim.sigset, NULL);
		signal(SIGKILL, sigterm);
		signal(SIGTERM, sigterm);
		signal(SIGINT, sigint);
		signal(SIGHUP, sighup);
		if (!lim.fgmode) {
			syslog(LOG_INFO, "Daemon starting up... (%s)", PACKAGE_STRING);
		}
		while (!done) {
			struct timeval tv;
			tv.tv_sec = lim.interval;
			tv.tv_usec = 0;
			if (select(0, NULL, NULL, NULL, &tv) < 0) {
				if (!done) { /* watchout for interupted syscall */
					syslog(LOG_ERR, "Failed call select: %s", strerror(errno));
					continue;
				} else {
					break;
				}
			}
			if ((res = pmon_scan(&lim)) < 0) {
				syslog(LOG_ERR, "Error in process scanner");
				done = 1;
			}
		}
		if (!lim.fgmode) {
			syslog(LOG_INFO, "Daemon exiting (%s)", PACKAGE_STRING);
		}
		closelog();
		return res == 0 ? 0 : 1;
	} else {
		if (pmon_scan(&lim) < 0) {
			return 1;
		}
	}

	return 0;
}

