#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include "db_funcs.h"
#include "log_funcs.h"

void make_media(media_t* new, medium_t type, const char* name, const char* location) {
  new->code = code_gen(type,name);
  new->type = type;
  strcpy(new->name,name);
  strcpy(new->location,location);
  new->update = time(NULL);
}

void make_book(book_t* new, uint32_t code, medium_t type, genre_t genre, 
	       const char* isbn, const char* title, const char* author_last,
	       const char* author_first, const char* author_rest) {
  new->code = code;
  new->type = type;
  new->genre = genre;
  strcpy(new->isbn,isbn);
  strcpy(new->title,title);
  strcpy(new->author_last,author_last);
  strcpy(new->author_first,author_first);
  strcpy(new->author_rest,author_rest);
}

void make_movie(movie_t* new, uint32_t code, medium_t type, genre_t genre,
		const char* title, const char* director, const char* studio, 
		short rating) {
  new->code = code;
  new->type = type;
  new->genre = genre;
  strcpy(new->title,title);
  strcpy(new->director,director);
  strcpy(new->studio,studio);
  new->rating = rating;
}

/* program in some big obivious section markers */

int main(int argc, char** argv) {
  int retval;
  int exit = 0;
  media_t test_values[5];
  media_t fetch_test;
  media_t* search_test = NULL;
  uint32_t num_results;
  media_t tc_test;
  book_t test_book;
  movie_t test_movie[2];
  book_t fetch_book_test;
  movie_t fetch_movie_test;
  book_t* search_test_book = NULL;
  movie_t* search_test_movie = NULL;

  setlocale(LC_ALL,"");
  init_access_log(NULL,STD_ERR_LOG,10);
  init_debug_log(NULL,STD_ERR_LOG,10);

  if (argc <= 1)
    retval = init_db("./test.db");
  else
    retval = init_db(argv[1]);

  if (retval != MI_EXIT_OK) {
    printf("init_db(): %s\n",error_string(retval));
    return 1;
  }

  /* test store objects */
  printf("Storing in objects: \n\n");
  make_media(&test_values[0], book, "THE MYSTERY OF THE BLUE TRAIN", "OFFICE");
  make_book (&test_book, test_values[0].code, book, mystery, "0-425-13026-6",
	     "THE MYSTERY OF THE BLUE TRAIN","CHRISTIE","AGATHA","");
  make_media(&test_values[1], dvd, "CATCH ME IF YOU CAN", "OFFICE");
  make_movie(&test_movie[0], test_values[1].code, dvd, drama|action, "CATCH ME IF YOU CAN", 
	     "STEVEN SPIELBERG", "DREAMWORKS", 7);
  make_media(&test_values[2], bluray, "HOW TO TRAIN YOUR DRAGON", "DEN");
  make_movie(&test_movie[1], test_values[2].code, bluray, fantasy|adventure|animation,
	     "HOW TO TRAIN YOUR DRAGON", "DEAN DEBLOIS",
	     "DREAMWORKS", 10);
  make_media(&test_values[3], xbox360, "HALO REACH", "DEN");
  make_media(&test_values[4], ps3, "GOD OF WAR 3", "DEN");

  for (int i = 0; i < 5; i++) {
    retval =  store(&test_values[i]);
    printf("store(): %s\n",error_string(retval));
    if (retval == MI_EXIT_ERROR) exit = 1;
  }

  retval = store_book(&test_book);
  printf("store_book(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK)
    return 1;

  retval = store_movie(&test_movie[0]);
  printf("store_movie(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK)
    return 1;

  retval = store_movie(&test_movie[1]);
  printf("store_movie(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK)
    return 1;

  /* test fetch objects */
  printf("Fetching objects out: \n\n");

  for (int i = 0; i < 5; i++) {
    retval = fetch(&fetch_test,test_values[i].code);
    printf("fetch(): %s\n",error_string(retval));
    if (retval == MI_EXIT_ERROR) {
      exit = 1;
      continue;
    }
    printf("#%u:\n",fetch_test.code);
    printf("\tType:     %s\n",medium_string(fetch_test.type));
    printf("\tName:     %s\n",fetch_test.name);
    printf("\tLocation: %s\n",fetch_test.location);
    printf("\tUpdate:   %jd\n",(intmax_t)fetch_test.update);
  }
  if (exit) return 1;

  retval = fetch_book(&fetch_book_test,test_book.code);
  printf("fetch_book(): %s\n",error_string(retval));
  if (retval == MI_EXIT_ERROR)
    return 1;
  printf("#%u:\n",fetch_book_test.code);
  printf("\tType:   %s\n",medium_string(fetch_book_test.type));
  printf("\tGenre:  %s\n",genre_string(fetch_book_test.genre));
  printf("\tISBN:   %s\n",fetch_book_test.isbn);
  printf("\tAuthor: %s, %s\n",fetch_book_test.author_last,fetch_book_test.author_first);
  printf("\tOther:  %s\n", fetch_book_test.author_rest);

  retval = fetch_movie(&fetch_movie_test,test_movie[0].code);
  printf("fetch_movie(): %s\n",error_string(retval));
  if (retval == MI_EXIT_ERROR)
    return 1;
  printf("#%u:\n",fetch_movie_test.code);
  printf("\tType:     %s\n",medium_string(fetch_movie_test.type));
  printf("\tGenre:    %s\n",genre_string(fetch_movie_test.genre));
  printf("\tTitle:    %s\n",fetch_movie_test.title);
  printf("\tDirector: %s\n",fetch_movie_test.director);
  printf("\tStudio:   %s\n",fetch_movie_test.studio);
  printf("\tRating:   %d\n",fetch_movie_test.rating);

  retval = fetch_movie(&fetch_movie_test,test_movie[1].code);
  printf("fetch_movie(): %s\n",error_string(retval));
  if (retval == MI_EXIT_ERROR)
    return 1;
  printf("#%u:\n",fetch_movie_test.code);
  printf("\tType:     %s\n",medium_string(fetch_movie_test.type));
  printf("\tGenre:    %s\n",genre_string(fetch_movie_test.genre));
  printf("\tTitle:    %s\n",fetch_movie_test.title);
  printf("\tDirector: %s\n",fetch_movie_test.director);
  printf("\tStudio:   %s\n",fetch_movie_test.studio);
  printf("\tRating:   %d\n",fetch_movie_test.rating);

  /* test search */
  printf("Searching for items: \n\n");

  retval = search(search_test,&num_results,"location = 'DEN'");
  if (retval != MI_EXIT_OK) {
    printf("search(): %s\n",error_string(retval));
    return 1;
  }

  if (num_results == 3)
    printf("search(): test ok\n");
  else
    printf("search(): %u results found",num_results);

  free(search_test);

  retval = search_books(search_test_book,&num_results,"author_last = 'HAMILTON'");
  printf("search_books(): %s\n",error_string(retval));
  printf("search_books(): %u results found\n",num_results);
  if (search_test_book) free(search_test_book);

  retval = search_movies(search_test_movie,&num_results,"rating = 10");
  printf("search_movies(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK) {
    printf("search_movies(): %s\n",error_string(retval));
    return 1;
  }

  if (num_results == 1)
    printf("search_movies(): test ok\n");
  if (search_test_movie) free(search_test_movie);

  /* test touch and checkout */
  printf("Testing touch and checkout: \n\n");

  make_media(&tc_test, other, "THE OBJECT THAT MOVES", "AETHERIUS");
  retval = store(&tc_test);
  printf("store(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK) return 1;

  retval = fetch(&fetch_test,tc_test.code);
  printf("fetch(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK) return 1;
  printf("#%u: %jd\n",fetch_test.code,(intmax_t)fetch_test.update);

  retval = touch(tc_test.code);
  printf("touch(): %s\n",error_string(retval));
  retval = checkout(tc_test.code,"NIRN");
  printf("checkout(): %s\n",error_string(retval));

  retval = fetch(&fetch_test,tc_test.code);
  printf("fetch(): %s\n",error_string(retval));
  if (retval != MI_EXIT_OK) return 1;
  printf("#%u: %s %jd\n",fetch_test.code,fetch_test.location,fetch_test.update);

  /* test csv dump */
  printf("Staring csv dump: \n\n");
  retval = csv_dump("./","test-");
  printf("csv_dump(): %s\n",error_string(retval));

  /* test pretty dump */
  printf("Starting pretty dump: \n\n");
  retval = pretty_dump("./test-ppd.txt");
  printf("pretty_dump(): %s\n",error_string(retval));

  retval = close_db();
  printf("close_db(): %s\n",error_string(retval));

  printf("All Done!\n");

  return 0;
}
