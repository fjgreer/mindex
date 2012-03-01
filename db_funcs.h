#ifndef __DB_FUNCS_H__
#define __DB_FUNCS_H__

/* db_funcs.h - part of mindex
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
/* exit codes */
#define MI_EXIT_ERROR -1
#define MI_EXIT_OK     0
#define MI_NO_RESULTS  1
#define MI_NOT_IMPL   -2
#define MI_EXISTS      2

/* typedefs */

/* type codes */
/* TODO: don't hardcode these */
typedef enum {
  book,
  xbox,
  xbox360,
  ps2,
  ps3,
  psp,
  n64,
  wii,
  ds,
  dvd,
  bluray,
  vhs,
  tape,
  cdrom,
  vinyl,
  other
} medium_t;

/* genre codes: */
typedef enum {
  reference   =     1, //0
  classic     =     2, //1
  religious   =     4, //2
  scifi       =     8, //3
  fantasy     =    16, //4
  mystery     =    32, //5
  fiction     =    64, //6
  computer    =   128, //7
  documentary =   256, //8
  action      =   512, //9
  adventure   =  1024, //10
  animation   =  2048, //11
  drama       =  4096, //12
  suspense    =  8192, //13
  thriller    = 16384, //14
  horror      = 32768, //15
  misc        = 65536, //16
  bmovie      = 131072 //17
} genre_t;

/* type for main index */
typedef struct {
  uint32_t code;          /* column 0 */
  medium_t type;          /* column 1 */
  char     name[121];     /* column 2 */
  char     location[121]; /* column 3 */
  time_t   update;        /* column 4 */
} media_t;

/* book type */
typedef struct {
  uint32_t  code;             /* column 0 */
  medium_t  type;             /* column 1 */
  genre_t   genre;            /* column 2 */
  char      isbn[61];         /* column 3 - 13 digit ISBN (utf8 0-9 are 1 byte) + NULL */
  char      title[121];       /* column 4 */
  char      author_last[61];  /* column 5 */
  char      author_first[91]; /* column 6 */
  char      author_rest[121]; /* column 7 - for books with multiple authors */
} book_t;

/* movie type */
typedef struct {
  uint32_t  code;          /* column 0 */
  medium_t  type;          /* column 1 */
  genre_t   genre;         /* column 2 */
  char      title[121];    /* column 3 */
  char      director[121]; /* column 4 */
  char      studio[121];   /* column 5 */
  short     rating;        /* column 6 */
} movie_t;

/* access functions */
int init_db(const char* file);
int close_db();

int fetch        (media_t* sought,uint32_t code); /* this fetches from the main database
						   * books and movies have more detail
						   * and can be looked up with the next two
						   * functions
						   */
int fetch_book   (book_t* sought,uint32_t code);  /* fetches books */ 
int fetch_movie  (movie_t* sought,uint32_t code); /* fetches movies */

int exists       (uint32_t code);                 /* because in this implementation, fetch()
						   * is crowded enough already
						   */
int exists_book  (uint32_t code);
int exists_movie (uint32_t code);

int store        (media_t* item);
int store_book   (book_t* item);
int store_movie  (movie_t* item);

int update       (media_t* item);
int update_book  (book_t* item);
int update_movie (movie_t* item);

int search       (media_t* items, uint32_t* num_results, const char* terms);
int search_books (book_t* items, uint32_t* num_results, const char* terms);
int search_movies(movie_t* items, uint32_t* num_results, const char* terms);

int delete       (uint32_t code);                 /* amazingly, only 1 of these is needed
						   * (it automagically looks up if its a movie
						   *  or book and deletes that entry)
						   */
int touch        (uint32_t code);                 /* updates time */
int checkout     (uint32_t code, const char* location); /* update time and location */

/* public errata functions */
uint32_t code_gen(medium_t type, const char* name);
int csv_load     (const char* dir, const char* prefix);
int csv_dump     (const char* dir, const char* prefix);
int pretty_dump  (const char* file);
const char* medium_string(medium_t type);
const char* genre_string(genre_t genre);
const char* error_string(int err);
const char* time_string(time_t time);

#endif /* __DB_FUNCS_H__ */
