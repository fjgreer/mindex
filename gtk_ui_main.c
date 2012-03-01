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
  g_object_unref(G_OBJET(builder));

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
  GtkTreeIter* iter;
  media_t* media;
  uint32_t* num_results;
  int retval;

  /* create the object */
  main = gtk_list_store_new(5,              /* columns */
			    G_TYPE_INT,     /* code */
			    G_TYPE_STRING,  /* type */
			    G_TYPE_STRING,  /* name */
			    G_TYPE_STRING,  /* location */
			    G_TYPE_STRING); /* update */

  /* aquire an iterator */
  gtk_tree_store_append(main, &iter, NULL);

  /* load the media */
  retval = search(media, &num_results, NULL);

  if (retval == MI_NO_RESULTS) {
    /* no results, new database? */
    g_free(G_OBJECT(iter));
    return main;
  }
  else if (retval != MI_EXIT_OK) {
    error_message("An error occured, please check your logs");
    g_free(G_OBJECT(iter));
    g_free(G_OBJECT(main));
    return NULL;
  }

  for (int i = 0; i < num_results; i++) {
