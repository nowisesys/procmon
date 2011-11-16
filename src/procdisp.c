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
 * File:   procdisp.c
 * Author: andlov
 *
 * Created on den 15 november 2011, 20:43
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <errno.h>

#include "procmon.h"
#include "procdisp.h"

#define pmon_bool(val) ((val) != 0 ? "yes" : "no")

void pmon_dump(const struct proc_limit *lim)
{
	debug(1, "Options:");
	debug(1, "---------------------------------------------------");
	debug(1, "      Executable: %s\t[exename] (command filter)", lim->exename);
	debug(1, "       CPU limit: %lu\t[nsexec] (seconds)", lim->nsexec);
	debug(1, "    Command line: %d\t[cmdline] (use long command line)", lim->cmdline);
	debug(1, "       Daemonize: %s\t[daemon]", pmon_bool(lim->daemon));
	debug(1, "           Debug: %s\t[debug]", pmon_bool(lim->debug));
	debug(1, "         Verbose: %s\t[verbose]", pmon_bool(lim->verbose));
	debug(1, " Foreground mode: %s\t[fgmode]", pmon_bool(lim->fgmode));
	debug(1, "           Fuzzy: %s\t[fuzzy] (use fuzzy filtering)", pmon_bool(lim->fuzzy));
	debug(1, "   Poll interval: %d\t[interval] (seconds)", lim->interval);
	debug(1, "          Signal: %d (%s)\t[signal]", lim->signal, strsignal(lim->signal));
	debug(1, "          Script: %s\t[script]", lim->script);
	debug(1, "Ticks per second: %d\t[ticks] (sysconf)", lim->ticks);
	debug(1, "         Dry-run: %s\t[dryrun]", pmon_bool(lim->dryrun));
	debug(1, "        PID file: %s\t[pidfile]", lim->pidfile);
	debug(1, "         User ID: %d (%d)\t[euid (ruid)]", lim->euid, lim->ruid);
	debug(1, "        Group ID: %d (%d)\t[egid (rgid)]", lim->egid, lim->rgid);
}

void pmon_disp(const struct proc_limit *lim, proc_t *pinf)
{
	debug(1, "---------------------------------------------------");
	debug(1, "                 Task ID: %d\t[tid] (POSIX thread ID)", pinf->tid); /* special */
	debug(1, "      Parent process PID: %d\t[ppid]", pinf->ppid); /* stat, status */
	debug(1, "              %%CPU usage: %d\t[pcpu]", pinf->pcpu); /* stat (special) */
	debug(1, "           Process state: %c\t[state]", pinf->state); /* stat, status */
	debug(1, "    Accumulated CPU time: %llu\t[utime] (user mode (jiffies))", pinf->utime); /* stat */
	debug(1, "    Accumulated CPU time: %llu\t[stime] (kernel mode (jiffies))", pinf->stime); /* stat */
	debug(1, "        Cumulative utime: %llu\t[cutime] (process and reaped children (jiffies))", pinf->cutime); /* stat */
	debug(1, "        Cumulative stime: %llu\t[cstime] (process and reaped children (jiffies))", pinf->cstime); /* stat */
	debug(1, "              Start time: %llu\t[start_time] (since epoch 1970-01-01)", pinf->start_time); /* stat */
	debug(1, "     Scheduling priority: %lu\t[priority] (kernel)", pinf->priority); /* stat */
	debug(1, "         UNIX nice level: %lu\t[nice]", pinf->nice); /* stat */
	debug(1, "      Real-time priority: %lu\t[rtprio]", pinf->rtprio); /* stat */
	debug(1, "        Scheduling class: %lu\t[sched]", pinf->sched); /* stat */
	debug(1, "          Effective user: %s (%d)\t[euser (euid)]", pinf->euser, pinf->euid); /* stat, status */
	debug(1, "               Real user: %s (%d)\t[ruser (ruid)]", pinf->ruser, pinf->ruid); /* status */
	debug(1, "              Saved user: %s (%d)\t[suser (suid)]", pinf->suser, pinf->suid); /* status */
	debug(1, "         Filesystem user: %s (%d)\t[fuser (fuid)]", pinf->fuser, pinf->fuid); /* status */
	debug(1, "         Effective group: %s (%d)\t[egroup (egid)]", pinf->egroup, pinf->egid); /* stat, status */
	debug(1, "              Real group: %s (%d)\t[rgroup (rgid)]", pinf->rgroup, pinf->rgid); /* status */
	debug(1, "             Saved group: %s (%d)\t[sgroup (sgid)]", pinf->sgroup, pinf->sgid); /* status */
	debug(1, "        Filesystem group: %s (%d)\t[fgroup (fgid)]", pinf->fgroup, pinf->fgid); /* status */
	debug(1, "        Process group ID: %d\t[pgrp]", pinf->pgrp); /* stat */
	debug(1, "              Session ID: %d\t[session]", pinf->session); /* stat */
	debug(1, "       Number of threads: %d\t[nlwp]", pinf->nlwp); /*stat,status*/
	debug(1, "           Task group ID: %d\t[tgid] (the POSIX PID, see also tid)", pinf->tgid); /* special */
	debug(1, "                     TTY: %d\t[tty] (full device number of controlling terminal)", pinf->tty); /* stat */
	debug(1, "    Terminal process GID: %d\t[tpgid]", pinf->tpgid); /* stat */
	debug(1, "             Current CPU: %d\t[processor] (most recent)", pinf->processor); /* stat */
}
