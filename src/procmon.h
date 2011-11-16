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
                unsigned long nscurr; /* current process cpu time */
                int signal; /* send signal */
                int daemon; /* daemonize */
                int fgmode; /* don't detach from controlling terminal */
                int cmdline; /* use command line */
                int interval; /* poll interval */
                int debug; /* debug mode */
                int verbose; /* be more verbose */
                int flags; /* openproc flags */
                int ticks; /* clock ticks per second */
                int fuzzy; /* fuzzy match command name */
                sigset_t sigset;
        };

        /*
         * Scan processes, killing runaway processes.
         */
        int pmon_scan(struct proc_limit *lim);

#ifdef	__cplusplus
}
#endif

#endif	/* PROCMON_H */

