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
 * File:   procmon.h
 * Author: andlov
 *
 * Created on den 15 november 2011, 04:15
 */

#ifndef PROCMON_H
#define	PROCMON_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef HAVE_PROC_READPROC_H
#include <proc/readproc.h>
#endif

#define PMON_TIMEOUT_INTERVAL 60        /* poll every minute by default */
#define PMON_DEFAULT_SIGNAL   SIGTERM   /* default signal to send */
#define PMON_DEFAULT_NSEXEC   3600      /* default number of CPU seconds */
#define PMON_DEFAULT_PIDFILE "/var/run/procmond.pid"

#define PMON_SECURE_INIT 1      /* set initial credentials */
#define PMON_SECURE_SCAN 2      /* setup credentials for scanning */
#define PMON_SECURE_REST 3      /* restore credentials after scanning */
#define PMON_SECURE_DONE 4      /* restore credentials at exit */

        extern int done; /* daemon exit flag */

        /*
         * Process scanning and application options.
         */
        struct proc_limit
        {
                const char *prog; /* this program name (short) */
                const char *self; /* this program name (argv) */
                const char *exename; /* executable (filter) */
                const char *cmdname; /* executable (process) */
                unsigned long nsexec; /* limit number of sec */
                unsigned long nscurr; /* current process CPU time */
                uid_t ruid; /* process real user ID */
                gid_t rgid; /* process real group ID */
                uid_t euid; /* process effective user ID */
                gid_t egid; /* process effective group ID */
                int secure; /* drop real user and group ID */
                int signal; /* send signal */
                int daemon; /* daemonize */
                char pidbuff[7]; /* buffer for daemon PID */
                const char *pidfile; /* write (daemon) PID to this location */
                const char *script; /* the script to run */
                int fgmode; /* don't detach from controlling terminal */
                int cmdline; /* use command line */
                int interval; /* poll interval */
                int debug; /* debug mode */
                int verbose; /* be more verbose */
                int flags; /* openproc flags */
                int ticks; /* clock ticks per second */
                int fuzzy; /* fuzzy match command name */
                sigset_t sigset; /* signal proc mask */
                int dryrun; /* only monitor and report */
        };

        /*
         * Set process privileges for operation.
         */
        int pmon_secure(const struct proc_limit *lim, int operation);

        /*
         * Scan processes, killing runaway processes.
         */
        int pmon_scan(struct proc_limit *lim);

#ifdef	__cplusplus
}
#endif

#endif	/* PROCMON_H */

