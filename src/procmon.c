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
 * File:   procmon.c
 * Author: andlov
 *
 * Created on den 15 november 2011, 04:15
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_LIBCAP
#include <sys/capability.h>
#endif
#include <sys/wait.h>
#include <errno.h>

#include "procmon.h"
#include "procdisp.h"

#define PMON_SKIP_KERNEL_THREAD 0
#define PMON_SKIP_FILTER_NO_MATCH 1

static const char * pmon_skip_msg[] = {
	"possibly a kernel thread",
	"filter don't match"
};

#define PMON_TIME_SHOW_HOURS   1
#define PMON_TIME_SHOW_MINUTES 2
#define PMON_TIME_SHOW_SECONDS 3

int done = 0;

struct pmon_time {
	unsigned short hours;
	unsigned short minutes;
	unsigned short seconds;
};

static int pmon_time_get(unsigned long time, struct pmon_time *res)
{
	res->hours = time / 3600;
	res->minutes = (time % 3600) / 60;
	res->seconds = (time % 60);

	if (res->hours) {
		return PMON_TIME_SHOW_HOURS;
	} else if (res->minutes) {
		return PMON_TIME_SHOW_MINUTES;
	} else {
		return PMON_TIME_SHOW_SECONDS;
	}
}

int pmon_secure(const struct proc_limit *lim, int operation)
{
	switch (operation) {
	case PMON_SECURE_INIT:

#ifdef HAVE_LIBCAP
		if (lim->euid != lim->ruid || lim->egid != lim->rgid) {
			cap_t caps;
			cap_value_t cap_list[2];

			if (lim->euid != lim->ruid) {
				if (!CAP_IS_SUPPORTED(CAP_SETUID)) {
					error("Set UID capability is unsupported");
					return -1;
				}
				cap_list[0] = CAP_SETUID;
			}
			if (lim->egid != lim->rgid) {
				if (!CAP_IS_SUPPORTED(CAP_SETGID)) {
					error("Set GID capability is unsupported");
					return -1;
				}
				cap_list[1] = CAP_SETGID;
			}

			if (!(caps = cap_get_proc())) {
				error("Failed call cap_get_proc (%s)", strerror(errno));
				return -1;
			}
			if (cap_set_flag(caps, CAP_EFFECTIVE, 2, cap_list, CAP_SET) < 0) {
				error("Failed call cap_set_flag (%s)", strerror(errno));
				return -1;
			}
			if (cap_set_proc(caps) < 0) {
				error("Failed call cap_set_proc (%s)", strerror(errno));
				return -1;
			}
			if (cap_free(caps) < 0) {
				error("Failed call cap_free (%s)", strerror(errno));
				return -1;
			}
		}
#endif

		if (lim->secure) {
			if (setgid(lim->egid) < 0) {
				error("Failed call setgid(%d) (%s)", lim->egid, strerror(errno));
				return -1;
			}
			if (setuid(lim->euid) < 0) {
				error("Failed call setuid(%d) (%s)", lim->euid, strerror(errno));
				return -1;
			}
		} else {
			if (setegid(lim->egid) < 0) {
				error("Failed call setegid(%d) (%s)", lim->egid, strerror(errno));
				return -1;
			}
			if (seteuid(lim->euid) < 0) {
				error("Failed call seteuid(%d) (%s)", lim->euid, strerror(errno));
				return -1;
			}
		}
		debug(3, "Credentials (init): (uid=%d, gid=%d [effective]) (uid=%d, gid=%d [real])",
			geteuid(), getegid(), getuid(), getgid());
		break;
	case PMON_SECURE_SCAN:
	case PMON_SECURE_DONE:
		if (!lim->secure) {
			if (seteuid(lim->ruid) < 0) {
				error("Failed call seteuid(%d) (%s)", lim->ruid, strerror(errno));
				return -1;
			}
			if (setegid(lim->rgid) < 0) {
				error("Failed call setegid(%d) (%s)", lim->rgid, strerror(errno));
				return -1;
			}
			debug(3, "Credentials (scan): (uid=%d, gid=%d [effective]) (uid=%d, gid=%d [real])",
				geteuid(), getegid(), getuid(), getgid());
		}
		break;
	case PMON_SECURE_REST:
		if (!lim->secure) {
			if (setegid(lim->egid) < 0) {
				error("Failed call setegid(%d) (%s)", lim->egid, strerror(errno));
				return -1;
			}
			if (seteuid(lim->euid) < 0) {
				error("Failed call seteuid(%d) (%s)", lim->euid, strerror(errno));
				return -1;
			}
			debug(3, "Credentials (rest): (uid=%d, gid=%d [effective]) (uid=%d, gid=%d [real])",
				geteuid(), getegid(), getuid(), getgid());
		}
		break;
	}

	return 0;
}

static void pmon_exec(const char *script, const struct proc_limit *lim, proc_t *pinf)
{
	char command[PATH_MAX];

	snprintf(command, sizeof(command), "%s %d %s", script, pinf->tid, pinf->cmd);
	if (system(command) < 0) {
		error("Failed execute %s (%s)", command, strerror(errno));
	}
}

static void pmon_skip(const struct proc_limit *lim, proc_t *pinf, int msg)
{
	debug(2, "Skipped process %s (pid=%d) [%s]", pinf->cmd, pinf->tid, pmon_skip_msg[msg]);
}

static int pmon_check(struct proc_limit *lim, proc_t *pinf)
{
	struct pmon_time time;

	if (lim->cmdline) {
		lim->cmdname = pinf->cmdline ? pinf->cmdline[0] : NULL;
	} else {
		lim->cmdname = pinf->cmd;
	}

	if (!lim->cmdname) {
		pmon_skip(lim, pinf, PMON_SKIP_KERNEL_THREAD);
		return 0;
	}

	if (strcmp(lim->self, lim->cmdname) == 0) {
		return 0; /* Prevent suicide ;-) */
	}
	if (lim->exename) {
		if (lim->verbose) {
			debug(2, "Looking for %s in %s", lim->exename, lim->cmdname);
		}
		if (lim->fuzzy) {
			if (!strstr(lim->cmdname, lim->exename)) {
				pmon_skip(lim, pinf, PMON_SKIP_FILTER_NO_MATCH);
				return 0;
			}
		} else {
			if (strcmp(lim->exename, lim->cmdname) != 0) {
				pmon_skip(lim, pinf, PMON_SKIP_FILTER_NO_MATCH);
				return 0;
			}
		}
	}

	if (lim->verbose) {
		info("Checking process %s (pid=%d)", lim->cmdname, pinf->tid);
		pmon_disp(lim, pinf);
	}

	/*
	 * The utime and stime is in jiffies. These should be converted to 
	 * seconds by dividing with ticks per seconds (from sysconf). 
	 * 
	 * I'm not sure this is true. For an kernel configured with 1000 Hz,
	 * sysconf is still returnning 100 as ticks per second, but it should
	 * be 1000.
	 */
	lim->nscurr = ((pinf->utime + pinf->stime) / lim->ticks);

	switch (pmon_time_get(lim->nscurr, &time)) {
	case PMON_TIME_SHOW_HOURS:
		debug(1, "Execution time (pid=%d): %lu seconds (%02d:%02d:%02d) [hh:mm:ss]",
			pinf->tid,
			lim->nscurr,
			time.hours,
			time.minutes,
			time.seconds);
		break;
	case PMON_TIME_SHOW_MINUTES:
		debug(1, "Execution time (pid=%d): %lu seconds (%02d:%02d) [mm:ss]",
			pinf->tid,
			lim->nscurr,
			time.minutes,
			time.seconds);
		break;
	case PMON_TIME_SHOW_SECONDS:
		debug(1, "Execution time (pid=%d): %lu seconds",
			pinf->tid,
			lim->nscurr);
		break;
	}

	if (lim->nscurr > lim->nsexec) {
		int status;

		notice("Process %d (%s) has exceeded CPU time limit %lu seconds (%lu sec).",
			pinf->tid, pinf->cmd, lim->nsexec, lim->nscurr);
		if (lim->dryrun) {
			return 0; /* be done here! */
		}
		if (lim->script) {
			pmon_exec(lim->script, lim, pinf);
		}
		notice("Sending signal %d (%s) to process %d.",
			lim->signal, strsignal(lim->signal), pinf->tid);
		if (kill(pinf->tid, lim->signal) < 0) {
			error("Failed send signal %d to process %d (%s)",
				lim->signal, pinf->tid, strerror(errno));
			return -1;
		}
		if (lim->signal == 0) {
			return 0;
		}
		if (waitpid(pinf->tid, &status, 0) < 0) {
			if (errno != ECHILD) {
				error("Failed waitpid on pid=%d (%s)",
					pinf->tid, strerror(errno));
				return -1;
			}
		} else if (WIFEXITED(status)) {
			info("Process %d has exited with code %d", pinf->tid, WEXITSTATUS(status));
		}
	}

	return 0;
}

int pmon_scan(struct proc_limit *lim)
{
	PROCTAB *ptab;
	proc_t *pinf;

	lim->flags = PROC_FILLARG | PROC_FILLSTAT | PROC_FILLSTATUS;

	if (lim->verbose && lim->debug) {
		lim->flags |= PROC_FILLUSR | PROC_FILLGRP | PROC_FILLMEM;
	}

	if (pmon_secure(lim, PMON_SECURE_SCAN) < 0) {
		exit(1);
	}

	if (!(ptab = openproc(lim->flags))) {
		error("Failed call openproc (%s)", strerror(errno));
		return -1;
	}

	while ((pinf = readproc(ptab, NULL))) {
		if (pmon_check(lim, pinf) < 0) {
			break;
		}
	}
	closeproc(ptab);

	if (pmon_secure(lim, PMON_SECURE_REST) < 0) {
		exit(1);
	}

	return 0;
}
