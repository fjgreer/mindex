#ifndef __LOG_FUNCS_H__
#define __LOG_FUNCS_H__

/* log_funcs.h - part of mindex
 *
 * Data structures and protoyping for logging facility
 * if log_access is used unitialized it is a no-op.
 * if log_debug is used unitialized it is logged to stderr.
 *
 * Copyright © 2012 Frank Joseph Greer
 *
 *  mindex is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* defines */
#define STD_ERR_LOG 0
#define FILE_LOG    1
#define NOOP_LOG    2

/* severity flags */
/* debug */
#define FATAL 0
#define ERROR 1
#define TODO  2
#define INFO  3

/* access */
#define VIOL  0
#define EDITS 1
#define ALL   2

/* functions */
int  init_access_log(const char* filename, int flag, int level);
int  init_debug_log (const char* filename, int flag, int level);
void log_access     (int level, const char* message);
void log_debug      (int level, const char* message);
void close_access_log();
void close_debug_log();

#endif /* __LOG_FUNCS_H__ */
