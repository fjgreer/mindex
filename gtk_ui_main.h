#ifndef __GTK_UI_MAIN_H__
#define __GTK_UI_MAIN_H__

/* gtk_ui_main.h - part of mindex
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
#define GTK_BUILDER_XML "mindex.glade"
#define AUTHORS {					\
  "Frank Joseph Greer <fjgreer@perquisitoromnia.net",	\
    NULL;						\
  }
#define COPYRIGHT "Copyright © 2012 Frank Joseph Greer";
#define LICENSE GTK_LICENSE_GPL_3_0
#define VERSION "0.1"
#define COMMENTS "GTK+ Media Indexing Program"

/* typedefs */
typedef struct {
  GtkWidget* window;
  GtkWidget* statusbar;
  GtkWidget* tree_view;
  GtkListStore* main;
  GtkListStore* books;
  GtkListStore* movies;

  guint statusbar_context_id;
  gchar* filename;
} mindex_t;

/* prototypes */
/* window callback functions */
void on_window_destroy(GObject* object, mindex_t* mindex);
gboolean on_window_delete_event(GtkWidget* widget, GdkEvent* event, mindex_t* mindex);
void on_main_tree_selection_changed(GtkTreeSelection *selection, mindex_t* mindex);

/* file menu callback functions */
void on_open_menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex);
void on_recent_menu_item_activate(GtkRecentChooser *chooser, mindex_t* mindex);
void on_export_cvs_menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex);
void on_import_cvs__menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex);
void on_quit_menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex);

/* edit menu callback functions */

/* help menu callback functions */
void on_about_menu_item_activate(GtkMenuItem *menuitem, mindex_t* mindex);

/* helpers */
void error_message(const gchar* message);
gboolean init_app(mindex_t* mindex);
gchar* get_open_filename(mindex_t* mindex);
gchar* get_save_filename(mindex_t* mindex);
void load_database(mindex_t* mindex, gchar* filename);
void reset_default_status(mindex_t* mindex);
GtkListStore* load_main();
GtkListStore* load_books();
GtkListStore* load_movies();

#endif
