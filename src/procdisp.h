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
 * File:   procdisp.h
 * Author: andlov
 *
 * Created on den 15 november 2011, 20:43
 */

#ifndef PROCDISP_H
#define	PROCDISP_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef HAVE_PROC_READPROC_H
#include <proc/readproc.h>
#endif

#define logmsg(prio, fmt, args...) \
do { \
	if((lim)->daemon && !(lim)->fgmode) { \
		syslog((prio) , (fmt) , ##args); \
	} else if((prio) == LOG_DEBUG) { \
		printf("debug: " fmt " (%s:%d)\n" , ##args, __FILE__ , __LINE__ ); \
	} else if((prio) == LOG_ERR) { \
		fprintf(stderr , "%s: error: " fmt "\n" , (lim)->prog , ##args); \
	} else if((prio) == LOG_WARNING) { \
		fprintf(stderr , "%s: warning: " fmt "\n" , (lim)->prog , ##args); \
	} else { \
		printf("info: " fmt "\n" , ##args); \
	} \
} while(0)

#define   debug(level, fmt, args...) if(((lim)->debug) >= (level)) { logmsg(LOG_DEBUG , fmt , ##args); }
#define   error(fmt, args...) logmsg(LOG_ERR ,   fmt , ##args)
#define    info(fmt, args...) logmsg(LOG_INFO , fmt , ##args)
#define    warn(fmt, args...) logmsg(LOG_WARNING , fmt , ##args)
#define  notice(fmt, args...) logmsg(LOG_NOTICE , fmt , ##args)

        void pmon_dump(const struct proc_limit *lim);
        void pmon_disp(const struct proc_limit *lim, proc_t *pinf);

#ifdef	__cplusplus
}
#endif

#endif	/* PROCDISP_H */

