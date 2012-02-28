/* log_funcs.c - part of mindex
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

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "log_funcs.h"

/* global settings */
FILE* access_log = NULL;
FILE* debug_log = NULL;
int log_level_access = 10;
int log_level_debug = 10;
int log_mode_access = -1;
int log_mode_debug = -1;

int init_access_log(const char* filename, int flag, int level) {
  if ((filename == NULL) && (flag == FILE_LOG)) {
    perror("init_access_log(): NULL filename for file logging");
    return 1;
  }

  switch (flag) {
  case NOOP_LOG:
    access_log = NULL;
    log_mode_access = NOOP_LOG;
    break;
  case STD_ERR_LOG:
    access_log = stderr;
    log_mode_access = STD_ERR_LOG;
    break;
  case FILE_LOG:
    access_log = fopen(filename,"a");
    if (access_log == NULL) {
      perror("init_access_log(): could not open filename");
      return 1;
    }
    log_mode_access = FILE_LOG;
    break;
  default:
    access_log = NULL;
    log_mode_access = NOOP_LOG;
  }
    log_level_access = level;
    return 0;
}

int init_debug_log(const char* filename, int flag, int level) {
  if ((filename == NULL) && (flag == FILE_LOG)) {
    perror("init_debug_log(): NULL filename for file logging");
    return 1;
  }

  switch (flag) {
  case NOOP_LOG:
    debug_log = NULL;
    log_mode_debug = NOOP_LOG;
    break;
  case STD_ERR_LOG:
    debug_log = stderr;
    log_mode_debug = STD_ERR_LOG;
    break;
  case FILE_LOG:
    debug_log = fopen(filename,"a");
    if (debug_log == NULL) {
      perror("init_debug_log(): could not open filename");
      return 1;
    }
    log_mode_debug = FILE_LOG;
    break;
  default:
    debug_log = NULL;
    log_mode_debug = NOOP_LOG;
  }
    log_level_debug = level;
    return 0;
}

void log_access(int level, const char* message) {
  if (log_mode_access == -1) return;
  if (log_mode_access == NOOP_LOG) return;
  if (level > log_level_access) return;

  fprintf(access_log,"%jd:%s\n",(intmax_t)time(NULL),message);
}

void log_debug(int level, const char* message) {
  char str_level[20];

  if (log_mode_debug == -1) {
    debug_log = stderr;
  }
  if (log_mode_debug == NOOP_LOG) return;
  if (level > log_level_debug) return;

  switch (level) {
  case FATAL:
    strcpy(str_level,"FATAL");
    break;
  case ERROR:
    strcpy(str_level,"ERROR");
    break;
  case TODO:
    strcpy(str_level,"TODO");
    break;
  case INFO:
    strcpy(str_level,"INFO");
    break;
  default:
    strcpy(str_level,"UNKWN");
  }

  fprintf(debug_log,"%jd:%s:%s\n",(intmax_t)time(NULL),str_level,message);
}

void close_access_log() {
  if (log_mode_access == FILE_LOG) fclose(access_log);
  access_log = NULL;
  log_level_access = 10;
  log_mode_access = -1;
}

void close_debug_log() {
  if (log_mode_debug == FILE_LOG) fclose(debug_log);
  debug_log = NULL;
  log_level_debug = 10;
  log_mode_debug = -1;
}
