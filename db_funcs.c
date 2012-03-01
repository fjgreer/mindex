/* db_funcs.c - part of mindex
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

/* great list of includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <string.h>
#include "db_funcs.h"
#include "log_funcs.h"

/* this space reserved for the great evil of global variables */
sqlite3* db_handle;

int init_db(const char* file) {
  /* schema defs */
  const char init_string_main[] =
    "CREATE TABLE IF NOT EXISTS main "
    "(code INTEGER PRIMARY KEY, type INTEGER NOT NULL, name TEXT NOT NULL, "
    "location TEXT NOT NULL, update_time INTEGER NOT NULL)";
  const char init_string_book[] =
    "CREATE TABLE IF NOT EXISTS books "
    "(code INTEGER PRIMARY KEY, type INTEGER NOT NULL, genre INTEGER NOT NULL, "
    "isbn TEXT, title TEXT, author_last TEXT, author_first TEXT, author_rest TEXT)";
  const char init_string_movie[] =
    "CREATE TABLE IF NOT EXISTS movies "
    "(code INTEGER PRIMARY KEY, type INTEGER NOT NULL, genre INTEGER NOT NULL, "
    "title TEXT, director TEXT, studio TEXT, rating INTEGER)";
  char buffer[512];
 
 /* open db */
 sprintf(buffer,"init_db(): opening %s as db file",file);
 log_debug(INFO,buffer);

 if (sqlite3_open(file, &db_handle) != SQLITE_OK) {
   log_debug(ERROR,"init_db(): error opening database");
   log_debug(ERROR,sqlite3_errmsg(db_handle));
   return MI_EXIT_ERROR;
 }
 log_debug(INFO,"init_db(): open successful");

 log_debug(INFO,"init_db(): db init (if needed)");

 /* exec main create */
 if (sqlite3_exec(db_handle,init_string_main,NULL,NULL,NULL) != SQLITE_OK) {
   log_debug(ERROR,"init_db(): sqlite exec error");
   log_debug(ERROR,sqlite3_errmsg(db_handle));
   return MI_EXIT_ERROR;
 }

 /* exec book create */
 if (sqlite3_exec(db_handle,init_string_book,NULL,NULL,NULL) != SQLITE_OK) {
   log_debug(ERROR,"init_db(): sqlite exec error");
   log_debug(ERROR,sqlite3_errmsg(db_handle));
   return MI_EXIT_ERROR;
 }

 /* exec movie create */
 if (sqlite3_exec(db_handle,init_string_movie,NULL,NULL,NULL) != SQLITE_OK) {
   log_debug(ERROR,"init_db(): sqlite exec error");
   log_debug(ERROR,sqlite3_errmsg(db_handle));
   return MI_EXIT_ERROR;
 }

 return MI_EXIT_OK;
}

int close_db() {
  sqlite3_close(db_handle);
  return MI_EXIT_OK;
}

int fetch(media_t* sought,uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;

  if (sought == NULL) {
    return exists(code);
  }
  /* select media with matching code
   * should only be one, so we'll only use the first result, if there is one
   */
  sprintf(buffer,"SELECT * FROM main WHERE code=%u",code);
  log_debug(INFO,"fetch(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"fetch(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    sought->code =          (uint32_t)sqlite3_column_int64(query,0);
    sought->type =          (medium_t)sqlite3_column_int(query,1);
    strcpy(sought->name,    (char *)sqlite3_column_text(query,2));
    strcpy(sought->location,(char *)sqlite3_column_text(query,3));
    sought->update =        (time_t)sqlite3_column_int64(query,4);
    log_debug(INFO,"fetch(): result found");
  }
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"fetch(): no results found");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"fetch(): some error didst occur");
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  sqlite3_finalize(query);
  return MI_EXIT_OK;
}

int fetch_book(book_t* sought,uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;

  if (sought == NULL) {
    return exists_book(code);
  }
  /* select media with matching code
   * should only be one, so we'll only use the first result, if there is one
   */
  sprintf(buffer,"SELECT * FROM books WHERE code=%u",code);
  log_debug(INFO,"fetch_book(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"fetch_book(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    sought->code =            (uint32_t)sqlite3_column_int64(query,0);
    sought->type =            (medium_t)sqlite3_column_int(query,1);
    sought->genre =          (genre_t)sqlite3_column_int(query,2);
    strcpy(sought->isbn,        (char *)sqlite3_column_text(query,3));
    strcpy(sought->title,       (char *)sqlite3_column_text(query,4));
    strcpy(sought->author_last, (char *)sqlite3_column_text(query,5));
    strcpy(sought->author_first,(char *)sqlite3_column_text(query,6));
    strcpy(sought->author_rest, (char *)sqlite3_column_text(query,7));
    log_debug(INFO,"fetch_book(): result found");
  }
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"fetch_book(): no results found");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"fetch_book(): some error didst occur");
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  sqlite3_finalize(query);
  return MI_EXIT_OK;
}

int fetch_movie(movie_t* sought,uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;

  if (sought == NULL) {
    return exists_movie(code);
  }
  /* select media with matching code
   * should only be one, so we'll only use the first result, if there is one
   */
  sprintf(buffer,"SELECT * FROM movies WHERE code=%u",code);
  log_debug(INFO,"fetch_movie(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"fetch_movie(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    sought->code =         (uint32_t)sqlite3_column_int64(query,0);
    sought->type =         (medium_t)sqlite3_column_int(query,1);
    sought->genre =       (genre_t)sqlite3_column_int(query,2);
    strcpy(sought->title,   (char *)sqlite3_column_text(query,3));
    strcpy(sought->director,(char *)sqlite3_column_text(query,4));
    strcpy(sought->studio,  (char *)sqlite3_column_text(query,5));
    sought->rating =         (short)sqlite3_column_int(query,6);
    log_debug(INFO,"fetch_movie(): result found");
  }
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"fetch_movie(): no results found");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"fetch_movie(): some error didst occur");
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  sqlite3_finalize(query);
  return MI_EXIT_OK;
}

int exists(uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;
  int count;

  sprintf(buffer,"SELECT COUNT(*) FROM main WHERE code = %u",code);
  log_debug(INFO,"exists(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"exists(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  
  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    count = (int)sqlite3_column_int(query,0);
    log_debug(INFO,"exists(): success");
  }
  else if (retval == SQLITE_DONE) {
    count = 0;
    log_debug(INFO,"exists(): failure");
  }
  else {
    log_debug(ERROR,"exists(): some error didst occur");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  return (count) ? MI_EXISTS : MI_NO_RESULTS;
}

int exists_book(uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;
  int count;

  sprintf(buffer,"SELECT COUNT(*) FROM books WHERE code = %u",code);
  log_debug(INFO,"exists_book(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"exists_book(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  
  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    count = (int)sqlite3_column_int(query,0);
    log_debug(INFO,"exists_book(): success");
  }
  else if (retval == SQLITE_DONE) {
    count = 0;
    log_debug(INFO,"exists_book(): failure");
  }
  else {
    log_debug(ERROR,"exists_book(): some error didst occur");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  return (count) ? MI_EXISTS : MI_NO_RESULTS;
}
      
int exists_movie(uint32_t code) {
  char buffer[128];
  sqlite3_stmt* query = NULL;
  int retval;
  int count;

  sprintf(buffer,"SELECT COUNT(*) FROM movies WHERE code = %u",code);
  log_debug(INFO,"exists_movie(): starting query");
  log_debug(INFO,buffer);
  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"exists_movie(): error with lookup");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  
  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW) {
    count = (int)sqlite3_column_int(query,0);
    log_debug(INFO,"exists_movie(): success");
  }
  else if (retval == SQLITE_DONE) {
    count = 0;
    log_debug(INFO,"exists_movie(): failure");
  }
  else {
    log_debug(ERROR,"exists_movie(): some error didst occur");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  return (count) ? MI_EXISTS : MI_NO_RESULTS;
}
  
int store(media_t* item) {
  char buffer[1024];
  int retval;

  retval = exists(item->code);
  if (retval != MI_NO_RESULTS) {
    return retval;
  }

  item->update=time(NULL);

  sprintf(buffer,"INSERT INTO main VALUES (%u, %d, '%s', '%s', %jd)",
	  item->code,item->type,item->name,item->location,(intmax_t)item->update);
  log_debug(INFO,"store(): executing query");
  log_debug(INFO,buffer);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    log_debug(ERROR,"store(): error with insertion");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  return MI_EXIT_OK;
}

int store_book(book_t* item) {
  char buffer[1024];
  int retval;

  retval = exists_book(item->code);
  if (retval != MI_NO_RESULTS) {
    return retval;
  }

  sprintf(buffer,"INSERT INTO books VALUES (%u, %d, %d, '%s', '%s', '%s', '%s', '%s')",
	  item->code,item->type,item->genre,item->isbn,item->title,item->author_last,
	  item->author_first,item->author_rest);
  log_debug(INFO,"store_book(): executing query");
  log_debug(INFO,buffer);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    log_debug(ERROR,"store_book(): error with insertion");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  return MI_EXIT_OK;
}

int store_movie(movie_t* item) {
  char buffer[1024];
  int retval;

  retval = exists_movie(item->code);
  if (retval != MI_NO_RESULTS) {
    return retval;
  }

  sprintf(buffer,"INSERT INTO movies VALUES (%u, %d, %d, '%s', '%s', '%s', %d)",
	  item->code,item->type,item->genre,item->title,item->director,
	  item->studio,item->rating);
  log_debug(INFO,"store_movie(): executing query");
  log_debug(INFO,buffer);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    log_debug(ERROR,"store_movie(): error with insertion");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  return MI_EXIT_OK;
}

int update(media_t* item) {
  char buffer[1024];
  char err_buf[128];
  int retval;

  /* check to see if item actually exists */
  retval = exists(item->code);

  if (retval == MI_EXISTS) {
    sprintf(err_buf,"update(): item #%u exists, starting update",item->code);
    log_debug(INFO,err_buf);
    item->update = time(NULL);
    sprintf(buffer,"UPDATE main SET type = %d, name = '%s', location = '%s', update_time = %jd WHERE code = %u",
	    item->type, item->name, item->location, item->update, item->code);
    log_debug(INFO,buffer);
    if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
      log_debug(ERROR,"update(): error with insertion");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      return MI_EXIT_ERROR;
    }
    return MI_EXIT_OK;
  }
  else {
    sprintf(err_buf,"update(): item #%u does not exist, or an error occured",item->code);
    log_debug(INFO,err_buf);
    return retval;
  }
}

int update_book(book_t* item) {
  char buffer[1024];
  char err_buf[128];
  int retval;

  /* check to see if item actually exists */
  retval = exists(item->code);

  if (retval == MI_EXISTS) {
    sprintf(err_buf,"update_book(): item #%u exists, starting update",item->code);
    log_debug(INFO,err_buf);
    sprintf(buffer,"UPDATE books SET type = %d, genre = %d, isbn = '%s', title = '%s', author_last = '%s', "
	    "author_first = '%s', author_rest = '%s' WHERE code = %u",
	    item->type,item->genre,item->isbn,item->title,item->author_last,item->author_first,
	    item->author_rest,item->code);
    log_debug(INFO,buffer);
    if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
      log_debug(ERROR,"update_book(): error with insertion");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      return MI_EXIT_ERROR;
    }
    return MI_EXIT_OK;
  }
  else {
    sprintf(err_buf,"update_book(): item #%u does not exist, or an error occured",item->code);
    log_debug(INFO,err_buf);
    return retval;
  }
}

int update_movie(movie_t* item) {
  char buffer[1024];
  char err_buf[128];
  int retval;

  /* check to see if item actually exists */
  retval = exists(item->code);

  if (retval == MI_EXISTS) {
    sprintf(err_buf,"update_movie(): item #%u exists, starting update",item->code);
    log_debug(INFO,err_buf);
    sprintf(buffer,"UPDATE movies SET type = %d, genre = %d, title = '%s', director = '%s', studio = '%s', rating = %d "
	    "WHERE code = %u", item->type, item->genre, item->title, item->director, item->studio, item->rating, 
	    item->code);
    log_debug(INFO,buffer);
    if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
      log_debug(ERROR,"update_movie(): error with insertion");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      return MI_EXIT_ERROR;
    }
    return MI_EXIT_OK;
  }
  else {
    sprintf(err_buf,"update_movie(): item #%u does not exist, or an error occured",item->code);
    log_debug(INFO,err_buf);
    return retval;
  }
}

int search(media_t* items, uint32_t* num_results, const char* terms) {
  char buffer[1024];
  sqlite3_stmt* query;
  int retval;
  uint32_t count;

  if (terms == NULL) {
    strcpy(buffer, "SELECT COUNT(*) FROM main");
  }
  else {
    if ((sizeof(buffer)-50) < sizeof(terms)) {
      log_debug(ERROR,"search(): terms exceeds buffer");
      return MI_EXIT_ERROR;
    }

    strcpy(buffer, "SELECT COUNT(*) FROM main WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW)
    *num_results = (uint32_t)sqlite3_column_int(query,0);
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"search(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"search(): unknown error");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  if (*num_results == 0) {
    log_debug(INFO,"search(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }

  if (terms == NULL) {
    strcpy(buffer, "SELECT * FROM main");
  }
  else {
    strcpy(buffer, "SELECT * FROM main WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  items = malloc(sizeof(media_t)*(*num_results));
  count = 0;

  log_debug(INFO,"search(): staring row processing");
  while (1) {
    retval = sqlite3_step(query);

    if (retval == SQLITE_ROW) {
      items[count].code =          (uint32_t)sqlite3_column_int(query,0);
      items[count].type =          (medium_t)sqlite3_column_int(query,1);
      strcpy(items[count].name,    (char *)sqlite3_column_text(query,2));
      strcpy(items[count].location,(char *)sqlite3_column_text(query,3));
      items[count].update =        (time_t)sqlite3_column_int64(query,4);
      count++;
      sprintf(buffer,"search(): loaded row %u", count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"search(): row processing done");
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* error of some sort */
      log_debug(ERROR,"search(): error during row processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      *num_results = 0;
      free(items);
      items = NULL;
      sqlite3_finalize(query);
      return MI_EXIT_ERROR;
    }
  }

  sqlite3_finalize(query);
  *num_results = count;
  return MI_EXIT_OK;
}

int search_books(book_t* items, uint32_t* num_results, const char* terms) {
  char buffer[1024];
  sqlite3_stmt* query;
  int retval;
  uint32_t count;

  if (terms == NULL) {
    strcpy(buffer, "SELECT COUNT(*) FROM books");
  }
  else {
    if ((sizeof(buffer)-50) < sizeof(terms)) {
      log_debug(ERROR,"search_books(): terms exceeds buffer");
      return MI_EXIT_ERROR;
    }

    strcpy(buffer, "SELECT COUNT(*) FROM books WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search_books(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search_books(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW)
    *num_results = (uint32_t)sqlite3_column_int(query,0);
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"search_books(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"search_books(): unknown error");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  if (*num_results == 0) {
    log_debug(INFO,"search(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }

  if (terms == NULL) {
    strcpy(buffer, "SELECT * FROM books");
  }
  else {
    strcpy(buffer, "SELECT * FROM books WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search_books(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search_books(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  items = malloc(sizeof(book_t)*(*num_results));
  count = 0;

  log_debug(INFO,"search_books(): staring row processing");
  while (1) {
    retval = sqlite3_step(query);

    if (retval == SQLITE_ROW) {
      items[count].code =            (uint32_t)sqlite3_column_int(query,0);
      items[count].type =            (medium_t)sqlite3_column_int(query,1);
      items[count].genre =          (genre_t)sqlite3_column_int(query,2);
      strcpy(items[count].isbn,        (char *)sqlite3_column_text(query,3));
      strcpy(items[count].title,       (char *)sqlite3_column_text(query,4));
      strcpy(items[count].author_last, (char *)sqlite3_column_text(query,5));
      strcpy(items[count].author_first,(char *)sqlite3_column_text(query,6));
      strcpy(items[count].author_rest, (char *)sqlite3_column_text(query,7));
      count++;
      sprintf(buffer,"search_books(): loaded row %u", count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"search_books(): row processing done");
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* error of some sort */
      log_debug(ERROR,"search_books(): error during row processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      *num_results = 0;
      free(items);
      items = NULL;
      sqlite3_finalize(query);
      return MI_EXIT_ERROR;
    }
  }

  sqlite3_finalize(query);
  *num_results = count;
  return MI_EXIT_OK;
}

int search_movies(movie_t* items, uint32_t* num_results, const char* terms) {
  char buffer[1024];
  sqlite3_stmt* query;
  int retval;
  uint32_t count;

  if (terms == NULL) {
    strcpy(buffer, "SELECT COUNT(*) FROM movies");
  }
  else {
    if ((sizeof(buffer)-50) < sizeof(terms)) {
      log_debug(ERROR,"search_movies(): terms exceeds buffer");
      return MI_EXIT_ERROR;
    }

    strcpy(buffer, "SELECT COUNT(*) FROM movies WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search_movies(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search_movies(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  retval = sqlite3_step(query);
  if (retval == SQLITE_ROW)
    *num_results = (uint32_t)sqlite3_column_int(query,0);
  else if (retval == SQLITE_DONE) {
    log_debug(INFO,"search_movies(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }
  else {
    log_debug(ERROR,"search_movies(): unknown error");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(query);
    return MI_EXIT_ERROR;
  }

  if (*num_results == 0) {
    log_debug(INFO,"search_movies(): no results for query");
    sqlite3_finalize(query);
    return MI_NO_RESULTS;
  }

  if (terms == NULL) {
    strpcy(buffer, "SELECT * FROM movies");
  }
  else {
    strcpy(buffer, "SELECT * FROM movies WHERE ");
    strcat(buffer, terms);
  }

  log_debug(INFO,"search_movies(): starting query");
  log_debug(INFO,buffer);

  if (sqlite3_prepare_v2(db_handle,buffer,-1,&query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"search_movies(): error executing query");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  items = malloc(sizeof(movie_t)*(*num_results));
  count = 0;

  log_debug(INFO,"search_movies(): staring row processing");
  while (1) {
    retval = sqlite3_step(query);

    if (retval == SQLITE_ROW) {
      items[count].code =         (uint32_t)sqlite3_column_int(query,0);
      items[count].type =         (medium_t)sqlite3_column_int(query,1);
      items[count].genre =       (genre_t)sqlite3_column_int(query,2);
      strcpy(items[count].title,   (char *)sqlite3_column_text(query,3));
      strcpy(items[count].director,(char *)sqlite3_column_text(query,4));
      strcpy(items[count].studio,  (char *)sqlite3_column_text(query,5));
      items[count].rating =         (short)sqlite3_column_int(query,6);
      count++;
      sprintf(buffer,"search_movies(): loaded row %u", count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"search_movies(): row processing done");
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* error of some sort */
      log_debug(ERROR,"search_movies: error during row processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%u rows processed out of %u rows actual",count,*num_results);
      log_debug(INFO,buffer);
      *num_results = 0;
      free(items);
      items = NULL;
      sqlite3_finalize(query);
      return MI_EXIT_ERROR;
    }
  }

  sqlite3_finalize(query);
  *num_results = count;
  return MI_EXIT_OK;
}

int delete(uint32_t code) {
  char buffer[128];

  sprintf(buffer,"delete(): starting of delete of #%u",code);
  log_debug(INFO,buffer);

  sprintf(buffer,"DELETE FROM main WHERE code = %u",code);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    sprintf(buffer,"delete(): delete of #%u failed",code);
    log_debug(ERROR,buffer);
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  sprintf(buffer,"DELETE FROM books WHERE code = %u",code);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    sprintf(buffer,"delete(): delete of #%u failed",code);
    log_debug(ERROR,buffer);
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  sprintf(buffer,"DELETE FROM movies WHERE code = %u",code);
  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    sprintf(buffer,"delete(): delete of #%u failed",code);
    log_debug(ERROR,buffer);
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  return MI_EXIT_OK;
}

int touch(uint32_t code) {
  char buffer[256];
  
  sprintf(buffer,"UPDATE main SET update_time = %jd WHERE code = %u",(intmax_t)time(NULL),code);
  log_debug(INFO,"touch():");
  log_debug(INFO,buffer);

  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    sprintf(buffer,"touch(): update of #%u failed",code);
    log_debug(ERROR,buffer);
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  return MI_EXIT_OK;
}

int checkout(uint32_t code, const char* location) {
  char buffer[256];

  sprintf(buffer,"UPDATE main SET location = '%s', update_time = %jd WHERE code = %u",
	  location,(intmax_t)time(NULL),code);
  log_debug(INFO,"checkout():");
  log_debug(INFO,buffer);

  if (sqlite3_exec(db_handle,buffer,NULL,NULL,NULL) != SQLITE_OK) {
    sprintf(buffer,"checkout(): update of #%u failed",code);
    log_debug(ERROR,buffer);
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }

  return MI_EXIT_OK;
}

uint32_t hash_string(char* source) {
  //DJB Hash
  uint32_t hash = 5381;

  for (int i = 0; i < (int)strlen(source); source++, i++)
    hash = ((hash << 5) + hash) + *source;

  return hash;
}

const char* medium_string(medium_t type) {
  static char name[30];
  switch (type) {
  case book:
    strcpy(name,"BOOK");
    break;
  case xbox:
    strcpy(name,"MICROSOFT XBOX");
    break;
  case xbox360:
    strcpy(name,"MICROSOFT XBOX 360");
    break;
  case ps2:
    strcpy(name,"SONY PLAYSTATION 2");
    break;
  case ps3:
    strcpy(name,"SONY PLAYSTATION 3");
    break;
  case psp:
    strcpy(name,"SONY PLAYSTATION PORTABLE");
    break;
  case n64:
    strcpy(name,"NINTENDO 64");
    break;
  case wii:
    strcpy(name,"NINTENDO WII");
    break;
  case ds:
    strcpy(name,"NINTENDO DS");
    break;
  case dvd:
    strcpy(name,"DVD");
    break;
  case bluray:
    strcpy(name,"BLU-RAY");
    break;
  case vhs:
    strcpy(name,"VHS");
    break;
  case tape:
    strcpy(name,"TAPE");
    break;
  case cdrom:
    strcpy(name,"CD-ROM");
    break;
  default:
    strcpy(name,"OTHER");
  }
  return name;
}

uint32_t code_gen(medium_t type, const char* name) {
  char buffer[151];
  strcat(buffer,medium_string(type));
  strcat(buffer,name);
  return hash_string(buffer);
}

int csv_load(const char* dir, const char* prefix) {
  log_debug(TODO,"csv_load(): not implemented");
  return MI_NOT_IMPL;
}

#define GENRE_STR_APP(flag,ptr,test,str) \
  do {					 \
  if ((flag & test) == test) {		 \
    if (comma_flag) {			 \
      strcpy(ptr,", ");			 \
      ptr += strlen(ptr);		 \
    }					 \
    strcpy(ptr,str);			 \
    ptr += strlen(ptr);			 \
    comma_flag++;			 \
  }					 \
  } while (0)

const char* genre_string(genre_t genre) {
  static char buffer[256];
  char* p;
  int comma_flag = 0;

  p = buffer;

  GENRE_STR_APP(genre,p,reference,"REFERENCE");
  GENRE_STR_APP(genre,p,classic,"CLASSIC");
  GENRE_STR_APP(genre,p,religious,"RELIGIOUS");
  GENRE_STR_APP(genre,p,scifi,"SCI-FI");
  GENRE_STR_APP(genre,p,fantasy,"FANTASY");
  GENRE_STR_APP(genre,p,mystery,"MYSTERY");
  GENRE_STR_APP(genre,p,fiction,"FICTION");
  GENRE_STR_APP(genre,p,computer,"COMPUTER");
  GENRE_STR_APP(genre,p,documentary,"DOCUMENTARY");
  GENRE_STR_APP(genre,p,action,"ACTION");
  GENRE_STR_APP(genre,p,adventure,"ADVENTURE");
  GENRE_STR_APP(genre,p,animation,"ANIMATION");
  GENRE_STR_APP(genre,p,drama,"DRAMA");
  GENRE_STR_APP(genre,p,suspense,"SUSPENSE");
  GENRE_STR_APP(genre,p,thriller,"THRILLER");
  GENRE_STR_APP(genre,p,horror,"HORROR");
  GENRE_STR_APP(genre,p,misc,"MISCELLANEOUS");
  GENRE_STR_APP(genre,p,bmovie,"B-MOVIE");

  return buffer;
}

const char* error_string(int err) {
  static char string[15];

  switch (err) {
  case MI_NOT_IMPL:
    strcpy(string,"MI_NOT_IMPL");
    break;
  case MI_EXIT_ERROR:
    strcpy(string,"MI_EXIT_ERROR");
    break;
  case MI_EXIT_OK:
    strcpy(string,"MI_EXIT_OK");
    break;
  case MI_NO_RESULTS:
    strcpy(string,"MI_NO_RESULTS");
    break;
  case MI_EXISTS:
    strcpy(string,"MI_EXISTS");
    break;
  default:
    strcpy(string,"UNKNOWN");
  }
  return string;
}

int csv_dump(const char* dir, const char* prefix) {
  FILE* main_out;
  FILE* book_out;
  FILE* movie_out;
  char  buffer[256];
  int retval,count;
  sqlite3_stmt* main_query;
  sqlite3_stmt* book_query;
  sqlite3_stmt* movie_query;
  media_t mtemp;
  book_t btemp;
  movie_t vtemp;

  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM main",-1,&main_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"csv_dump(): could not query all from main");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  
  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM books",-1,&book_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"csv_dump(): could not query all from books");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(main_query);
    return MI_EXIT_ERROR;
  }

  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM movies",-1,&movie_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"csv_dump(): could not query all from movies");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    return MI_EXIT_ERROR;
  }


  log_debug(INFO,"csv_dump(): opening files for write");
  /* concat the filename for the main table dump*/
  strcpy(buffer,dir);
  strcat(buffer,prefix);
  strcat(buffer,"main.csv");
  log_debug(INFO,"csv_dump(): attempting to open:");
  log_debug(INFO,buffer);

  main_out = fopen(buffer,"w");
  if (!main_out) {
    log_debug(ERROR,"csv_dump(): could not open file for write, giving up");
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    sqlite3_finalize(movie_query);
    return MI_EXIT_ERROR;
  }

  /* concat the filename for the book table dump*/
  strcpy(buffer,dir);
  strcat(buffer,prefix);
  strcat(buffer,"book.csv");
  log_debug(INFO,"csv_dump(): attempting to open:");
  log_debug(INFO,buffer);

  book_out = fopen(buffer,"w");
  if (!book_out) {
    log_debug(ERROR,"csv_dump(): could not open file for write, giving up");
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    sqlite3_finalize(movie_query);
    return MI_EXIT_ERROR;
  }

  /* concat teh filename for the movie table dump */
  strcpy(buffer,dir);
  strcat(buffer,prefix);
  strcat(buffer,"movie.csv");
  log_debug(INFO,"csv_dump(): attempting to open:");
  log_debug(INFO,buffer);

  movie_out = fopen(buffer,"w");
  if (!movie_out) {
    log_debug(ERROR,"csv_dump(): could not open file for write, giving up");
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    sqlite3_finalize(movie_query);
    return MI_EXIT_ERROR;
  }

  /* dump the main table */
  log_debug(INFO,"csv_dump(): beginning main table dump");

  fprintf(main_out,"mindex dump main\n");
  fprintf(main_out,"%jd\n",(intmax_t)time(NULL));
  fprintf(main_out,"---begin---\n");
  fprintf(main_out,"code,type,name,location,update\n");
   
  log_debug(INFO,"csv_dump(): processing rows main");
  count = 0;
  while (1) {
    retval = sqlite3_step(main_query);

    if (retval == SQLITE_ROW) {
      mtemp.code =        (uint32_t)sqlite3_column_int(main_query,0);
      mtemp.type =        (medium_t)sqlite3_column_int(main_query,1);
      strcpy(mtemp.name,    (char *)sqlite3_column_text(main_query,2));
      strcpy(mtemp.location,(char *)sqlite3_column_text(main_query,3));
      mtemp.update =        (time_t)sqlite3_column_int64(main_query,4);
      count++;
      fprintf(main_out,"%u,%d,%s,%s,%jd\n",
	      mtemp.code,mtemp.type,mtemp.name,mtemp.location,(intmax_t)mtemp.update);
      sprintf(buffer,"csv_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"csv_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"csv_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(main_out);
      fclose(movie_out);
      fclose(book_out);
      sqlite3_finalize(main_query);
      sqlite3_finalize(book_query);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }

  fclose(main_out);
  sqlite3_finalize(main_query);

  /* end main dump*/

  /* start book dump */
  log_debug(INFO,"csv_dump(): beginning book table dump");

  fprintf(book_out,"mindex dump book\n");
  fprintf(book_out,"%jd\n",(intmax_t)time(NULL));
  fprintf(book_out,"---begin---\n");
  fprintf(book_out,"code,type,genre,isbn,title,author_last,author_first,author_rest\n");
   
  log_debug(INFO,"csv_dump(): processing rows book");
  count = 0;
  while (1) {
    retval = sqlite3_step(book_query);

    if (retval == SQLITE_ROW) {
      btemp.code =            (uint32_t)sqlite3_column_int(book_query,0);
      btemp.type =            (medium_t)sqlite3_column_int(book_query,1);
      btemp.genre =            (genre_t)sqlite3_column_int(book_query,2);
      strcpy(btemp.isbn,        (char *)sqlite3_column_text(book_query,3));
      strcpy(btemp.title,       (char *)sqlite3_column_text(book_query,4));
      strcpy(btemp.author_last, (char *)sqlite3_column_text(book_query,5));
      strcpy(btemp.author_first,(char *)sqlite3_column_text(book_query,6));
      strcpy(btemp.author_rest, (char *)sqlite3_column_text(book_query,7));
      count++;
      fprintf(book_out,"%u,%d,%d,%s,%s,%s,%s,%s\n",
	      btemp.code,btemp.type,btemp.genre,btemp.isbn,btemp.title,btemp.author_last,
	      btemp.author_first,btemp.author_rest);
      sprintf(buffer,"csv_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"csv_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"csv_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(movie_out);
      fclose(book_out);
      sqlite3_finalize(book_query);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }

  fclose(book_out);
  sqlite3_finalize(book_query);

  /* end book dump */

  /* start movie dump */
  log_debug(INFO,"csv_dump(): beginning movie table dump");

  fprintf(movie_out,"mindex dump movie\n");
  fprintf(movie_out,"%jd\n",(intmax_t)time(NULL));
  fprintf(movie_out,"---begin---\n");
  fprintf(movie_out,"code,type,genre,title,director,studio,rating\n");
   
  log_debug(INFO,"csv_dump(): processing rows movie");
  count = 0;
  while (1) {
    retval = sqlite3_step(movie_query);

    if (retval == SQLITE_ROW) {
      vtemp.code =        (uint32_t)sqlite3_column_int(movie_query,0);
      vtemp.type =        (medium_t)sqlite3_column_int(movie_query,1);
      vtemp.genre =        (genre_t)sqlite3_column_int(movie_query,2);
      strcpy(vtemp.title,   (char *)sqlite3_column_text(movie_query,3));
      strcpy(vtemp.director,(char *)sqlite3_column_text(movie_query,4));
      strcpy(vtemp.studio,  (char *)sqlite3_column_text(movie_query,5));
      vtemp.rating =         (short)sqlite3_column_int(movie_query,6);
      count++;
      fprintf(movie_out,"%u,%d,%d,%s,%s,%s,%d\n",
	      vtemp.code,vtemp.type,vtemp.genre,vtemp.title,vtemp.director,vtemp.studio,vtemp.rating);
      sprintf(buffer,"csv_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"csv_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"csv_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(movie_out);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }

  fclose(movie_out);
  sqlite3_finalize(movie_query);

  /* end movie dump */

  return MI_EXIT_OK;
}

int pretty_dump(const char* file) {
  FILE* out;
  char  buffer[256];
  int retval,count;
  sqlite3_stmt* main_query;
  sqlite3_stmt* book_query;
  sqlite3_stmt* movie_query;
  media_t mtemp;
  book_t btemp;
  movie_t vtemp;
  struct tm* time_tmp;
  time_t cur_time;

  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM main",-1,&main_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"pretty_dump(): could not query all from main");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    return MI_EXIT_ERROR;
  }
  
  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM books",-1,&book_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"pretty_dump(): could not query all from books");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(main_query);
    return MI_EXIT_ERROR;
  }

  if (sqlite3_prepare_v2(db_handle,"SELECT * FROM movies",-1,&movie_query,NULL) != SQLITE_OK) {
    log_debug(ERROR,"pretty_dump(): could not query all from movies");
    log_debug(ERROR,sqlite3_errmsg(db_handle));
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    return MI_EXIT_ERROR;
  }


  log_debug(INFO,"pretty_dump(): opening file for write");
  log_debug(INFO,file);
  out = fopen(file,"w");
  if (!out) {
    log_debug(ERROR,"pretty_dump(): could not open file for write, giving up");
    sqlite3_finalize(main_query);
    sqlite3_finalize(book_query);
    sqlite3_finalize(movie_query);
    return MI_EXIT_ERROR;
  }

  /* dump the main table */
  log_debug(INFO,"pretty_dump(): beginning main table dump");
  cur_time = time(NULL);
  fprintf(out,"mindex dump main\n");
  time_tmp = gmtime(&cur_time);
  strftime(buffer,sizeof(buffer), "%F @ %T %Z", time_tmp);
  fprintf(out,"%s\n",buffer);
  fprintf(out,"---begin---\n");
   
  log_debug(INFO,"pretty_dump(): processing rows main");
  count = 0;
  while (1) {
    retval = sqlite3_step(main_query);

    if (retval == SQLITE_ROW) {
      mtemp.code =        (uint32_t)sqlite3_column_int(main_query,0);
      mtemp.type =        (medium_t)sqlite3_column_int(main_query,1);
      strcpy(mtemp.name,    (char *)sqlite3_column_text(main_query,2));
      strcpy(mtemp.location,(char *)sqlite3_column_text(main_query,3));
      mtemp.update =        (time_t)sqlite3_column_int64(main_query,4);
      count++;
      fprintf(out,"#%u: %s\n",mtemp.code,mtemp.name);
      fprintf(out,"\tType:        %s\n",medium_string(mtemp.type));
      fprintf(out,"\tLocation:    %s\n",mtemp.location);
      time_tmp = gmtime(&mtemp.update);
      strftime(buffer,sizeof(buffer), "%F @ %T %Z", time_tmp);
      fprintf(out,"\tLast Update: %s\n\n",buffer);

      sprintf(buffer,"pretty_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"pretty_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"pretty_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(out);
      sqlite3_finalize(main_query);
      sqlite3_finalize(book_query);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }
  fprintf(out,"---end main---\n\n");
  sqlite3_finalize(main_query);

  /* end main dump*/

  /* start book dump */
  log_debug(INFO,"pretty_dump(): beginning book table dump");

  fprintf(out,"mindex dump book\n");
  time_tmp = gmtime(&cur_time);
  strftime(buffer,sizeof(buffer), "%F @ %T %Z", time_tmp);
  fprintf(out,"%s\n",buffer);
  fprintf(out,"---begin---\n");
   
  log_debug(INFO,"pretty_dump(): processing rows book");
  count = 0;
  while (1) {
    retval = sqlite3_step(book_query);

    if (retval == SQLITE_ROW) {
      btemp.code =            (uint32_t)sqlite3_column_int(book_query,0);
      btemp.type =            (medium_t)sqlite3_column_int(book_query,1);
      btemp.genre =            (genre_t)sqlite3_column_int(book_query,2);
      strcpy(btemp.isbn,        (char *)sqlite3_column_text(book_query,3));
      strcpy(btemp.title,       (char *)sqlite3_column_text(book_query,4));
      strcpy(btemp.author_last, (char *)sqlite3_column_text(book_query,5));
      strcpy(btemp.author_first,(char *)sqlite3_column_text(book_query,6));
      strcpy(btemp.author_rest, (char *)sqlite3_column_text(book_query,7));
      count++;
      fprintf(out,"#%u:%s: %s\n",btemp.code,medium_string(btemp.type),btemp.isbn);
      fprintf(out,"\tTitle:  %s\n",btemp.title);
      fprintf(out,"\tAuthor: %s, %s\n",btemp.author_last,btemp.author_first);
      fprintf(out,"\tOther:  %s\n",btemp.author_rest);
      fprintf(out,"\tGenre:  %s\n\n",genre_string(btemp.genre));
	      
      sprintf(buffer,"pretty_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"pretty_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"pretty_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(out);
      sqlite3_finalize(book_query);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }

  fprintf(out,"---end book---\n\n");
  sqlite3_finalize(book_query);

  /* end book dump */

  /* start movie dump */
  log_debug(INFO,"pretty_dump(): beginning movie table dump");

  fprintf(out,"mindex dump movie\n");
  time_tmp = gmtime(&cur_time);
  strftime(buffer,sizeof(buffer), "%F @ %T %Z", time_tmp);
  fprintf(out,"%s\n",buffer);
  fprintf(out,"---begin---\n");
   
  log_debug(INFO,"pretty_dump(): processing rows movie");
  count = 0;
  while (1) {
    retval = sqlite3_step(movie_query);

    if (retval == SQLITE_ROW) {
      vtemp.code =        (uint32_t)sqlite3_column_int(movie_query,0);
      vtemp.type =        (medium_t)sqlite3_column_int(movie_query,1);
      vtemp.genre =        (genre_t)sqlite3_column_int(movie_query,2);
      strcpy(vtemp.title,   (char *)sqlite3_column_text(movie_query,3));
      strcpy(vtemp.director,(char *)sqlite3_column_text(movie_query,4));
      strcpy(vtemp.studio,  (char *)sqlite3_column_text(movie_query,5));
      vtemp.rating =         (short)sqlite3_column_int(movie_query,6);
      count++;
      fprintf(out,"#%u:%s: %s\n",vtemp.code,medium_string(vtemp.type),vtemp.title);
      fprintf(out,"\tDirector: %s\n",vtemp.director);
      fprintf(out,"\tStudio:   %s\n",vtemp.studio);
      fprintf(out,"\tGenre:    %s\n",genre_string(vtemp.genre));
      fprintf(out,"\tRating:   %d/10\n\n",vtemp.rating);

      sprintf(buffer,"pretty_dump(): output row %d",count);
      log_debug(INFO,buffer);
    }
    else if (retval == SQLITE_DONE) {
      /* all done */
      log_debug(INFO,"pretty_dump(): rows done");
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);
      break;
    }
    else {
      /* errors didst occur */
      log_debug(ERROR,"pretty_dump(): error during processing");
      log_debug(ERROR,sqlite3_errmsg(db_handle));
      sprintf(buffer,"%d rows processed",count);
      log_debug(INFO,buffer);

      fclose(out);
      sqlite3_finalize(movie_query);
      return MI_EXIT_ERROR;
    }
  }
  fprintf(out, "---movie end---\n");
  fclose(out);
  sqlite3_finalize(movie_query);

  /* end movie dump */

  return MI_EXIT_OK;
}
