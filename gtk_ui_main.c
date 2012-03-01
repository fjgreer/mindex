/* gtk_ui_main.c - part of mindex
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

#include <gtk/gtk.h>
#include <stdint.h>
#include "log_funcs.h"
#include "db_funcs.h"
#include "gtk_ui_main.h"

int main(int argc, char** argv) {
  mindex_t* mindex;

  /* start the custom log facility */
  init_access_log(NULL, STD_ERR_LOG, 10);
  init_debug_log(NULL, STD_ERR_LOG, 10);

  /* allocate mindex object */
  mindex = g_slice_new(mindex_t);

  /* initialization */
  gtk_init(&argc, &argv);

  if (init_app(mindex) == FALSE) {
    log_debug(FATAL, "Could not initialize mindex, so sorry.");
    return 1;
  }

  /* show the window */
  gtk_widget_show(mindex->window);

  /* gtk loop */
  gtk_main();

  g_slice_free(mindex_t, mindex);

  return 0;
}

/* display an error message to user, and log to console */
void error_message(const gchar* message) {
  GtkWidget* dialog;

  /* log */
  log_debug(ERROR,message);

  /* create an error dialog */
  dialog = gtk_message_dialog_new(NULL,
				  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				  GTK_MESSAGE_ERROR,
				  GTK_BUTTONS_OK,
				  message);
  gtk_window_set_title(GTK_WINDOW(dialog), "An Error Didst Occur");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

gboolean init_app(mindex_t* mindex) {
  GtkBuilder* builder;
  GError* err = NULL;
  guint id;

  /* build interface from spec */
  builder = gtk_builder_new();
  if (gtk_builder_add_from_file(builder, GTK_BUILDER_XML, &err) == 0) {
    error_message(err->message);
    g_error_free(err);
    return FALSE;
  }

  /* store the parts we care about */
  mindex->window = GTK_WIDGET(gtk_builder_get_object(builder,"main_window"));
  mindex->statusbar = GTK_WIDGET(gtk_builder_get_object(builder,"main_window_statusbar"));
  mindex->tree_view = GTK_WIDGET(gtk_builder_get_object(builder,"main_tree_display"));

  /* connect signals */
  gtk_builder_connect_signals(builder, mindex);

  /* free gtkBuilder */
  g_object_unref(G_OBJECT(builder));

  /* set default icon to the GTK "index" icon */
  gtk_window_set_default_icon_name(GTK_STOCK_INDEX);

  /* setup and initialize the statusbar */
  id = gtk_statusbar_get_context_id(GTK_STATUSBAR(mindex->statusbar), "mindex - The Media Index");
  mindex->statusbar_context_id = id;
  reset_default_status(mindex);

  /* set filename to NULL since no database opened yet */
  mindex->filename = NULL;

  return TRUE;
}

/* open dialog */
gchar* get_open_filename(mindex_t* mindex) {
  GtkWidget *chooser;
  gchar* filename = NULL;

  chooser = gtk_file_chooser_dialog_new("Open File…",
					GTK_WINDOW(mindex->window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					NULL);
  if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));

  gtk_widget_destroy(chooser);
  return filename;
}

ghcar* get_save_filename(mindex_t* mindex) {
  GtkWidget *chooser;
  gchar* filename = NULL;

  chooser = gtk_file_chooser_dialog_new("Save File…",
					GTK_WINDOW(mindex->window),
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					NULL);

  if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_OK)
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));

  gtk_widget_destroy(chooser);
  return filename;
}

/* loads and/or create a new database */
void load_database(mindex_t* mindex, gchar* filename) {
  gchar* status;
  gchar* err;

  /* set status to loading */
  status = g_strdup_printf("Loading Database %s…", filename);
  gtk_statusbar_push(GTK_STATUSBAR(mindex->statusbar), mindex->statusbar_context_id, status);
  g_free(status);

  /* clear event queue */
  while (gtk_events_pending()) gtk_main_iteration();

  if (init_db(filename) != MI_EXIT_OK) {
    err = g_strdup_printf("Could not load %s", filename);
    error_message(err);
    g_free(err);
    g_free(filename);
  } else {
    mindex->main = load_main();
    mindex->books = load_books();
    mindex->movies = load_movies();
    if (mindex->filename != NULL) g_free(mindex->filename);
    mindex->filename = filename;
  }

  /* clear loading status and restore default  */
  gtk_statusbar_pop (GTK_STATUSBAR (editor->statusbar),
		     editor->statusbar_context_id);
  reset_default_status (editor);
}

void reset_default_status(mindex_t* mindex) {
  gchar* file;
  gchar* status;

  if (mindex->filename == NULL)
    file = g_strdup("No Database Opened…");
  else
    file = g_path_get_basename(mindex->filename);

  status = g_strdup_printf("Database: %s", file);
  gtk_statusbar_pop(GTK_STATUSBAR(mindex->statusbar), mindex->statusbar_context_id);
  gtk_statusbar_push(GTK_STATUSBAR(mindex->statusbar), mindex->statusbar_context_id, status);
  g_free(status);
  g_free(file);
}

GtkListStore* load_main() {
  GtkListStore* main;
  GtkTreeIter iter;
  media_t* media;
  uint32_t num_results;
  int retval;

  /* load the media */
  retval = search(media, &num_results, NULL);

  if (retval == MI_EXIT_ERROR) {
    error_message("An error occured, please check your logs");
    return NULL;
  }

  /* create the object */
  main = gtk_list_store_new(5,              /* columns  */
			    G_TYPE_INT,     /* code     */
			    G_TYPE_STRING,  /* type     */
			    G_TYPE_STRING,  /* name     */
			    G_TYPE_STRING,  /* location */
			    G_TYPE_STRING); /* update   */

  for (int i = 0; i < num_results; i++) {
    gtk_list_store_append(main, &iter);
    gtk_list_store_set(main, &iter,
		       0, media[i].code,
		       1, medium_string(media[i].type),
		       2, media[i].name,
		       3, media[i].location,
		       4, time_string(media[i].update),
		       -1);
  }

  free(media);
  return main;
}

GtkListStore* load_books() {
  GtkListStore* books;
  GtkTreeIter iter;
  book_t* book;
  uint32_t num_results;
  int retval;

  /* load the books */
  retval = search_books(book, &num_results, NULL);

  if (retval == MI_EXIT_ERROR) {
    error_message("An error occured, please check your logs");
    return NULL;
  }

  /* create the object */
  books = gtk_list_store_new(8,              /* columns      */
			     G_TYPE_INT,     /* code         */
			     G_TYPE_STRING,  /* type         */
			     G_TYPE_STRING,  /* genre        */
			     G_TYPE_STRING,  /* isbn         */
			     G_TYPE_STRING,  /* title        */
			     G_TYPE_STRING,  /* author_last  */
			     G_TYPE_STRING,  /* author_first */
			     G_TYPE_STRING); /* author_rest  */

  for (int i = 0; i < num_results; i++) {
    gtk_list_store_append(books, &iter);
    gkt_list_store_set(books, &iter,
		       0, book[i].code,
		       1, medium_string(book[i].type),
		       2, genre_string(book[i].genre),
		       3, book[i].isbn,
		       4, book[i].title,
		       5, book[i].author_last,
		       6, book[i].author_first,
		       7, book[i].author_rest,
		       -1);
  }

  free(book);
  return books;
}

GtkListStore* load_movies() {
  GtkListStore* movies;
  GtkTreeiter iter;
  movie_t* movie;
  uint32_t num_results;
  int retval;

  /* load the movies */
  retval = search_movies(movie, &num_results, NULL);

  if (retval == MI_EXIT_ERROR) {
    error_message("An error occured, please check your logs");
    return NULL;
  }

  /* create the object */
  movies = gtk_list_store_new(7,             /* columns  */
			      G_TYPE_INT,    /* code     */
			      G_TYPE_STRING, /* type     */
			      G_TYPE_STRING, /* genre    */
			      G_TYPE_STRING, /* title    */
			      G_TYPE_STRING, /* director */
			      G_TYPE_STRING, /* studio   */
			      G_TYPE_INT);   /* rating   */

  for (int i = 0; i < num_results, i++) {
    gtk_list_store_append(movies, &iter);
    gtk_list_store_set(movies, &iter,
		       0, movie[i].code,
		       1, medium_string(movie[i].type),
		       2, genre_string(movie[i].genre),
		       3, movie[i].title,
		       4, movie[i].director,
		       5, movie[i].studio,
		       6, movie[i].rating,
		       -1);
  }

  free(movie);
  return movies;
}
/*
void setup_tree_main() {
  GtkTreeViewColumn* column;
  GtkCellRenderer* renderer = gtk_cell_renderer_text_new();

  column = gtk_tree_view_column_new_with_attributes("Code", renderer,
						    "text", 0,
						    NULL);
}
*/
/* window callback functions */
void on_window_destroy(GObject* object, mindex_t* mindex) {
  gtk_main_quit();
}

gboolean on_window_delete_event(GtkWidget* widget, GdkEvent* event, mindex_t* mindex) {
  return FALSE;
}

void on_main_tree_selection_changed(GtkTreeSelection *selection, mindex_t* mindex) {
  log_debug(TODO, "on_main_tree_selection_changed_not_implemented");
}

/* file menu callback functions */
void on_open_menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex) {
  gchar* filename;

  filename = get_open_filename(mindex);

  if (filename != NULL) {
    gtk_tree_view_set_model(mindex->tree_view, NULL);
  
    /* check for existing data */
    if (mindex->main != NULL) g_object_unref(G_OBJECT(mindex->main));
    if (mindex->books != NULL) g_object_unref(G_OBJECT(mindex->books));
    if (mindex->movies != NULL) g_object_unref(G_OBJECT(mindex->movies));

    /* load new data */
    load_database(mindex, filename);

    /* display tree view */
    gtk_tree_view_set_model(mindex->tree_view, mindex->main);
    setup_tree_main();
  }
}
