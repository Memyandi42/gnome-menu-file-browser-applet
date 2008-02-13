/*
 * File:				preferences.c
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		January 2008
 * Last Modified by:	Axel von Bertoldi
 * (C) 2005-2008		Axel von Bertoldi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to:
 * The Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1301, USA.
 */

#include "preferences.h"
#include "utils.h"
#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <glib/gprintf.h>
#include <stdlib.h>

/******************************************************************************/
struct _AppletPreferencesPrivate {
	GtkWidget	*window;
	PanelApplet *applet;
	GtkWidget	*tree_view;
};
/******************************************************************************/
#define APPLET_PREFERENCES_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_APPLET_PREFERENCES, AppletPreferencesPrivate))
#define ICON_BUTTON_SIZE 24
/******************************************************************************/
enum  {
	APPLET_PREFERENCES_DUMMY_PROPERTY
};
enum {
	PREFS_CHANGED,
	LAST_SIGNAL
};
enum {
	RESPONSE_REVERT
};
enum
{
   LABEL_COLUMN,
   PATH_COLUMN,
   N_COLUMNS
};
/******************************************************************************/
static guint applet_preferences_signals[LAST_SIGNAL] = { 0 };
/* this is bad, but I need to get at the button from all over the file to
 * update it's state. The other option is make it private member. */
/*static GtkWidget *revert_button = NULL;*/
/******************************************************************************/
static gpointer applet_preferences_parent_class = NULL;
static void applet_preferences_dispose (GObject *obj);
/******************************************************************************/
static void
applet_preferences_on_show_icon_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *self = (AppletPreferences *)data;

	/* get the new state from the widget and update the prefs structure */
	self->menu_bar_prefs->show_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

	PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
	signal_data->signal_id = PREFS_SIGNAL_SHOW_ICON;
	signal_data->instance  = -1;
	signal_data->label 	  = NULL;
	signal_data->path 	  = NULL;

	/* emit the signal so the panel menu bar updates itself */
	g_signal_emit (G_OBJECT (data),
				   applet_preferences_signals [PREFS_CHANGED],
				   0,
				   signal_data);

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	return;
}
/******************************************************************************/
static void
applet_preferences_on_show_hidden_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *self = (AppletPreferences *)data;
	/* get the new state from the widget and update the prefs structure. No
	 * need to let the menu bar or browser object know */
	self->menu_bar_prefs->browser_prefs->show_hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	return;
}
/******************************************************************************/
static void
applet_preferences_on_terminal_changed (GtkWidget *widget, gpointer data) {
	gchar *tmp = NULL;
	AppletPreferences *self = (AppletPreferences *)data;
	
	/* get the new state from the widget and update the prefs structure. No
	 * need to let the menu bar or browser object know */
	tmp = self->menu_bar_prefs->browser_prefs->terminal;
	g_free (tmp);
	self->menu_bar_prefs->browser_prefs->terminal = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	return;
}	
/******************************************************************************/
static void
applet_preferences_on_icon_select (GtkWidget *button, gpointer data) {
	GtkWidget *file_chooser_dialog;
	AppletPreferences *self = (AppletPreferences *)data;
	/* set up a file chooser dialog so we can choose the new icon. An icon
	 * chooser would be better, but that widget is deprecated in the Gnome UI
	 * lib and GTK equivalent doesn't exist yet */
	file_chooser_dialog = gtk_file_chooser_dialog_new ("Select Icon",
													   NULL,
													   GTK_FILE_CHOOSER_ACTION_OPEN,
													   GTK_STOCK_CANCEL,
													   GTK_RESPONSE_CANCEL,
													   GTK_STOCK_OPEN,
													   GTK_RESPONSE_ACCEPT,
													   NULL);
	/* Set the starting path */
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_dialog),
								   self->menu_bar_prefs->icon);
	/* check the reply */
	if (gtk_dialog_run (GTK_DIALOG (file_chooser_dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *new_icon;
		new_icon = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_dialog));
		/* make sure the new icon is not the same as the old one */	
		if (g_ascii_strcasecmp (new_icon, self->menu_bar_prefs->icon)) {
			/* update the prefs structure */
			g_free (self->menu_bar_prefs->icon);
			self->menu_bar_prefs->icon = new_icon;

			/* update the button's icon */
			GtkWidget *icon = gtk_button_get_image (GTK_BUTTON (button));
			gtk_widget_destroy (icon);
			icon = utils_get_scaled_image_from_file (self->menu_bar_prefs->icon,
													 ICON_BUTTON_SIZE);
			gtk_button_set_image (GTK_BUTTON (button), icon);

			PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
			signal_data->signal_id = PREFS_SIGNAL_ICON_CHANGED;
			signal_data->instance  = -1;
			signal_data->label 	   = NULL;
			signal_data->path 	   = NULL;

			/* emit the signal so the panel menu bar updates itself */
			g_signal_emit (G_OBJECT (data),
						   applet_preferences_signals [PREFS_CHANGED],
						   0,
						   signal_data);
			/*gtk_widget_set_sensitive (revert_button, TRUE);*/
		}
		else {
			/* they chose the same icon again! */
			g_free (new_icon);
		}

	}
	/* clean up and update the state of the revert button. A pref has changed
	 * so the button should now be sensitive  */
	gtk_widget_destroy (file_chooser_dialog);
}
/******************************************************************************/
static void
applet_preferences_save_to_gconf (AppletPreferences *self) {
	PanelApplet *applet = self->priv->applet;
	GError *error = NULL;
	/* save the data in the prefs structure to the gconf schema (or whatever)*/
	/* show hidden files? */
	panel_applet_gconf_set_bool (applet,
								 KEY_HIDDEN_SHOW,
								 self->menu_bar_prefs->browser_prefs->show_hidden,
								 &error);
	utils_check_gerror (&error);
	/* terminal */
	panel_applet_gconf_set_string (applet,
								   KEY_TERMINAL,
								   self->menu_bar_prefs->browser_prefs->terminal,
								   &error);
	utils_check_gerror (&error);
	/* the icon */
	panel_applet_gconf_set_string (applet,
								   KEY_ICON_NAME,
								   self->menu_bar_prefs->icon,
								   &error);
	utils_check_gerror (&error);
	/* show the icon? */
	panel_applet_gconf_set_bool (applet,
								 KEY_ICON_SHOW,
								 self->menu_bar_prefs->show_icon,
								 &error);
	utils_check_gerror (&error);
	/* directory list */
	panel_applet_gconf_set_list (applet,
								 KEY_DIR,
								 GCONF_VALUE_STRING,
								 self->menu_bar_prefs->dirs,
								 &error);
	utils_check_gerror (&error);
	/* labels list */
	panel_applet_gconf_set_list (applet,
								 KEY_LABELS,
								 GCONF_VALUE_STRING,
								 self->menu_bar_prefs->labels,
								 &error);
	utils_check_gerror (&error);
	return;
}
/******************************************************************************/
static void
applet_preferences_dialog_response (GtkWidget *window, gint response, gpointer data) {
	AppletPreferences *self = (AppletPreferences *)data;
	/* figure out what button closed the dialog and take the appropriate action */
	switch (response) {
		case RESPONSE_REVERT:
			/* revert to the saved config */
			/*gtk_widget_set_sensitive (revert_button, FALSE);*/
			break;

		case GTK_RESPONSE_CLOSE:
			/* save the current configuration */
			applet_preferences_save_to_gconf (self);
			/*gtk_widget_set_sensitive (revert_button, FALSE);*/
			gtk_widget_hide (window);
			break;

		case GTK_RESPONSE_DELETE_EVENT:
			/* neither save nor revert */
			gtk_widget_hide (window);
			break;
	}
	return;
}
/******************************************************************************/
static void
applet_preferences_label_cell_edited (GtkCellRenderer	*cell,
									  gchar				*path_string,
									  gchar				*new_string,
									  gpointer			data){

	AppletPreferences	*self		= (AppletPreferences *)data;
	GtkWidget			*tree_view	= self->priv->tree_view;
	GtkTreeModel		*model;
	GtkTreeIter			iter;
	gchar				*path		= NULL;

	/* get the model where the actual data is stored */
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	/* get an iterator to the model for the currently selected cell */
	gtk_tree_model_get_iter_from_string (model,
										 &iter,
										 path_string);
	/* update the model */
	gtk_list_store_set (GTK_LIST_STORE (model), &iter,
						LABEL_COLUMN, new_string,
						-1);

	/* the path associated with the selection */
	gtk_tree_model_get (model,
						&iter,
						PATH_COLUMN, &path,
						-1);

	/* create the data structure with the event info to pass to panel_menu_bar */
	PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
	signal_data->signal_id = PREFS_SIGNAL_DIR_CHANGED;
	signal_data->instance  = atoi (path_string);
	signal_data->label 	   = g_strdup (new_string);
	signal_data->path 	   = path;

	/* update this item from the list holding the label prefs */
	GSList *tmp = NULL;
	tmp = g_slist_nth (self->menu_bar_prefs->labels, signal_data->instance);
	g_free (tmp->data);
	tmp->data = (gpointer)g_strdup (new_string);

	/* emit the signal so the panel menu bar updates itself */
	g_signal_emit (G_OBJECT (data),
				   applet_preferences_signals [PREFS_CHANGED],
				   0,
				   signal_data);

	/* update the revert button*/
	/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	return;
}
/******************************************************************************/
static void
applet_preferences_path_cell_activated (GtkTreeView		  *tree_view,
										GtkTreePath		  *path,
										GtkTreeViewColumn *col,
										gpointer		   data) {
	AppletPreferences	*self	= (AppletPreferences *)data;
	GtkTreeModel *model;
	GtkTreeIter	 iter;
	GtkWidget	 *file_chooser_dialog;
	gchar		 *old_path = NULL;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	/* get an iterator to the model for the currently selected cell */
	gtk_tree_model_get_iter (model,
							 &iter,
							 path);
	/* the "Path" value for the active cell */
	gtk_tree_model_get (model,
						&iter,
						PATH_COLUMN, &old_path,
						-1);

	/* make a file chooser object to select the new path */
	file_chooser_dialog = gtk_file_chooser_dialog_new ("Select New Folder To Browse",
													   NULL,
													   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
													   GTK_STOCK_CANCEL,
													   GTK_RESPONSE_CANCEL,
													   GTK_STOCK_OPEN,
													   GTK_RESPONSE_ACCEPT,
													   NULL);
	/* Set the starting path as the old path */
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_dialog),
								   old_path);
	g_free (old_path);
	/* run the dialog */
	if (gtk_dialog_run (GTK_DIALOG (file_chooser_dialog)) == GTK_RESPONSE_ACCEPT) {
		/* get the new path*/
		gchar *new_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_dialog));
		/* only update data if it's not the same as the old one */
		if (g_ascii_strcasecmp (old_path, new_path)) {
			gtk_list_store_set (GTK_LIST_STORE (model), &iter,
								PATH_COLUMN, new_path,
								-1);


			/* the label associated with the selection */
			gchar *label = NULL;
			gtk_tree_model_get (model,
								&iter,
								LABEL_COLUMN, &label,
								-1);

			/* get the instance from the iterator */
			gchar *instance = gtk_tree_model_get_string_from_iter (model, &iter);

			/* create the data structure with the event info to pass to panel_menu_bar */
			PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
			signal_data->signal_id = PREFS_SIGNAL_DIR_CHANGED;
			signal_data->instance  = atoi (instance);
			signal_data->label 	   = label;
			signal_data->path 	   = new_path;
			g_free (instance);

			/* update this item from the list holding the path prefs */
			GSList *tmp = NULL;
			tmp = g_slist_nth (self->menu_bar_prefs->dirs, signal_data->instance);
			g_free (tmp->data);
			tmp->data = (gpointer)g_strdup (new_path);
			
			/* emit the signal so the panel menu bar updates itself */
			g_signal_emit (G_OBJECT (data),
						   applet_preferences_signals [PREFS_CHANGED],
						   0,
						   signal_data);

			/* update the revert button*/
			/*gtk_widget_set_sensitive (revert_button, TRUE);*/
		}
	}
	gtk_widget_destroy (file_chooser_dialog);

	return;
}
/******************************************************************************/
static void
applet_preferences_on_add_dir_clicked (GtkWidget *widget, gpointer data) {
	GtkTreeModel *model;
	GtkTreeIter   iter;
	GtkWidget *file_chooser_dialog;
	AppletPreferences *self = (AppletPreferences *)data;
	GtkWidget *tree_view = self->priv->tree_view;

	file_chooser_dialog = gtk_file_chooser_dialog_new ("Select Folder To Add",
													   NULL,
													   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
													   GTK_STOCK_CANCEL,
													   GTK_RESPONSE_CANCEL,
													   GTK_STOCK_OPEN,
													   GTK_RESPONSE_ACCEPT,
													   NULL);
	/* Set the starting path */
	gchar *start_path = g_strdup_printf ("%s/*", g_get_home_dir ());
	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_dialog),
								   start_path);
	g_free (start_path);

	/* check the reply */
	if (gtk_dialog_run (GTK_DIALOG (file_chooser_dialog)) == GTK_RESPONSE_ACCEPT) {
		/* get the new path, and make the label from it */
		gchar *dir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_dialog));
		gchar *label = g_path_get_basename (dir);
		/* get the view's model, add a row and set the values */
		model = gtk_tree_view_get_model (GTK_TREE_VIEW(tree_view));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							LABEL_COLUMN, label,
							PATH_COLUMN, dir,
							-1);

		/* create the data structure with the event info to pass to panel_menu_bar */
		PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
		signal_data->signal_id = PREFS_SIGNAL_DIR_ADD;
		signal_data->instance  = -1;
		signal_data->label 	   = label;
		signal_data->path 	   = dir;

		/* add this item from the list holding the path/label prefs */
		self->menu_bar_prefs->dirs   = g_slist_append (self->menu_bar_prefs->dirs,   g_strdup (dir));
		self->menu_bar_prefs->labels = g_slist_append (self->menu_bar_prefs->labels, g_strdup(label));

		/* emit the signal so the panel menu bar updates itself */
		g_signal_emit (G_OBJECT (data),
					   applet_preferences_signals [PREFS_CHANGED],
					   0,
					   signal_data);

		/* update the revert button*/
		/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	}
	gtk_widget_destroy (file_chooser_dialog);
	return;
}
/******************************************************************************/
static void
applet_preferences_on_rem_dir_clicked (GtkWidget *widget, gpointer data) {
	GtkTreeSelection	*selection;
	GtkTreeModel		*model;
	GtkTreeIter			iter;
	AppletPreferences	*self = (AppletPreferences *)data;
	GtkWidget			*tree_view = self->priv->tree_view;

	/* get the current selection */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree_view));

	/* get the iterator for the current selection and remove the corresponding
	 * row form the model */
	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

		/* get the instance from the iterator */
		gchar *instance = gtk_tree_model_get_string_from_iter (model, &iter);

		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

		/* create the data structure with the event info to pass to panel_menu_bar */
		PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
		signal_data->signal_id = PREFS_SIGNAL_DIR_DEL;
		signal_data->instance  = atoi (instance);
		signal_data->label 	   = NULL;
		signal_data->path 	   = NULL;
		g_free (instance);

		/* remove this item from the list holding the path/label prefs */
		GSList *tmp = NULL;
		tmp = g_slist_nth (self->menu_bar_prefs->dirs, signal_data->instance);
		g_free (tmp->data);
		self->menu_bar_prefs->dirs = g_slist_delete_link (self->menu_bar_prefs->dirs, tmp);
		tmp = g_slist_nth (self->menu_bar_prefs->labels, signal_data->instance);
		g_free (tmp->data);
		self->menu_bar_prefs->labels = g_slist_delete_link (self->menu_bar_prefs->labels, tmp);

		/* emit the signal so the panel menu bar updates itself */
		g_signal_emit (G_OBJECT (data),
				applet_preferences_signals [PREFS_CHANGED],
				0,
				signal_data);

		/* update the revert button*/
		/*gtk_widget_set_sensitive (revert_button, TRUE);*/
	}

	return;
}
/******************************************************************************/
static void
applet_preferences_on_down_dir_clicked (GtkWidget *widget, gpointer data) {
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel 		*model = NULL;
	GtkTreeIter			iter, iter_next;
	AppletPreferences	*self = (AppletPreferences *)data;
	GtkWidget			*tree_view = self->priv->tree_view;

	/* get the current selection */
	selection =  gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	/* get the iterator for the current selection */
  	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		/* is this dangerous? Time will tell... */
		iter_next = iter;
		/* get the next iterator and it it's valid, swap them */
		if (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter_next)){

			/* get the instance from the iterator */
			gchar *instance = gtk_tree_model_get_string_from_iter (model, &iter);

			gtk_list_store_swap (GTK_LIST_STORE (model),
								 &iter,
								 &iter_next);

			/* create the data structure with the event info to pass to panel_menu_bar */
			PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
			signal_data->signal_id = PREFS_SIGNAL_DIR_MOVE_DOWN;
			signal_data->instance  = atoi (instance);
			signal_data->label 	   = NULL;
			signal_data->path 	   = NULL;
			g_free (instance);

			/* swap the entries in the list holding the path/label prefs */
			g_slist_swap_data (self->menu_bar_prefs->dirs, signal_data->instance);
			g_slist_swap_data (self->menu_bar_prefs->labels, signal_data->instance);

			/* emit the signal so the panel menu bar updates itself */
			g_signal_emit (G_OBJECT (data),
					applet_preferences_signals [PREFS_CHANGED],
					0,
					signal_data);

			/* update the revert button*/
			/*gtk_widget_set_sensitive (revert_button, TRUE);*/
		}
	}
	return;
}
/******************************************************************************/
static void
applet_preferences_on_up_dir_clicked (GtkWidget *widget, gpointer data) {
	GtkTreeSelection	*selection	= NULL;
	GtkTreeModel 		*model		= NULL;
	GtkTreeIter			iter, iter_prev;
	GtkTreePath			*path;
	AppletPreferences	*self		= (AppletPreferences *)data;
	GtkWidget			*tree_view	= self->priv->tree_view;

	/* get the current selection */
	selection =  gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	/* get the iterator for the current selection */
  	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		/* Sigh! gtk_tree_model_iter_prev does not exist, so have to get the
		 * path for that iter and do a prev on it, then get the iter for the
		 * new path. Shite!*/
		path = gtk_tree_model_get_path (model, &iter);
		/* if the previous path is valid */
		if (gtk_tree_path_prev (path)) {
			/* get the corresponding iter, and swap the store elements*/
			gtk_tree_model_get_iter (model, &iter_prev, path);

			/* get the instance from the iterator */
			gchar *instance = gtk_tree_model_get_string_from_iter (model, &iter);

			gtk_list_store_swap (GTK_LIST_STORE (model),
								 &iter,
								 &iter_prev);

			/* create the data structure with the event info to pass to panel_menu_bar */
			PrefsChangedSignalData *signal_data = g_new0 (PrefsChangedSignalData, 1);
			signal_data->signal_id = PREFS_SIGNAL_DIR_MOVE_UP;
			signal_data->instance  = atoi (instance);
			signal_data->label 	   = NULL;
			signal_data->path 	   = NULL;
			g_free (instance);

			/* swap the entries in the list holding the path/label prefs */
			g_slist_swap_data (self->menu_bar_prefs->dirs, signal_data->instance - 1);
			g_slist_swap_data (self->menu_bar_prefs->labels, signal_data->instance - 1);

			/* emit the signal so the panel menu bar updates itself */
			g_signal_emit (G_OBJECT (data),
						   applet_preferences_signals [PREFS_CHANGED],
						   0,
						   signal_data);

			/* update the revert button*/
			/*gtk_widget_set_sensitive (revert_button, TRUE);*/
		}
		gtk_tree_path_free (path);
	}
	return;
}
/******************************************************************************/
static void
applet_preferences_create_list_view (AppletPreferences *self) {
	GtkTreeIter			iter;
	GtkListStore		*store;
	GtkTreeViewColumn	*column;
	GtkCellRenderer		*renderer;
	GtkTreeSelection 	*selection;

	/* Create a model.  We are using the store model for now, though we
	* could use any other GtkTreeModel */
	store = gtk_list_store_new (N_COLUMNS,
							    G_TYPE_STRING,
							    G_TYPE_STRING);

	/* fill the model with data */
	GSList *tmp_dir   = self->menu_bar_prefs->dirs;
	GSList *tmp_label = self->menu_bar_prefs->labels;
	while (tmp_label && tmp_dir) {
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
							LABEL_COLUMN, (gchar *)tmp_label->data,
							PATH_COLUMN, (gchar *)tmp_dir->data,
							-1);
		tmp_dir   = tmp_dir->next;
		tmp_label = tmp_label->next;
	}

	/* Create a view */
	gtk_tree_view_set_model (GTK_TREE_VIEW (self->priv->tree_view), GTK_TREE_MODEL (store));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->priv->tree_view), TRUE);

	/* The view now holds a reference.  We can get rid of our own
	* reference */
	g_object_unref (G_OBJECT (store));

	/* Create a cell render for the label, make it editable as set the callback  */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer),
					  "edited",
					  G_CALLBACK (applet_preferences_label_cell_edited),
					  (gpointer) self);

	/* Create a column, associating the "text" attribute of the
	* cell_renderer to the first column of the model */
	column = gtk_tree_view_column_new_with_attributes ("Label", renderer,
													   "text", LABEL_COLUMN,
													   NULL);
	/* Add the column to the view. */
	gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->tree_view), column);

	/* Create a cell render for the path */
	renderer = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (self->priv->tree_view),
					  "row-activated",
					  G_CALLBACK (applet_preferences_path_cell_activated),
					  (gpointer) self);
	column = gtk_tree_view_column_new_with_attributes ("Path", renderer,
													   "text", PATH_COLUMN,
													   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->tree_view), column);

	/*put the selection in SINGLE mode */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(self->priv->tree_view));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	
	return;
}
/******************************************************************************/
void
applet_preferences_make_dialog (AppletPreferences *self) {
	g_return_if_fail (self == NULL || IS_APPLET_PREFERENCES (self));

	GtkWidget *window;
    GtkWidget *show_icon;
    GtkWidget *icon_button;
    GtkWidget *show_hidden;
    GtkWidget *terminal;
	MenuBarPrefs *mb_prefs = self->menu_bar_prefs;

	if (self->priv->window == NULL) {
		GladeXML *xml = glade_xml_new (GLADEUI_PATH, NULL, NULL);
		g_return_if_fail (xml != NULL);
		window = glade_xml_get_widget (xml, "preferences_dialog");

		g_signal_connect (G_OBJECT (GTK_DIALOG (window)),
						  "response",
						  G_CALLBACK (applet_preferences_dialog_response),
						  (gpointer) self);

		g_signal_connect (G_OBJECT (window),
						  "delete_event",
						  G_CALLBACK (gtk_widget_hide),
						  (gpointer) self);

		/***** terminal *****/
		terminal = glade_xml_get_widget (xml, "terminal_entry");
		gtk_entry_set_width_chars (GTK_ENTRY (terminal), 15);
		gtk_entry_set_text (GTK_ENTRY (terminal),
							mb_prefs->browser_prefs->terminal);
		g_signal_connect (G_OBJECT (terminal),			
						  "changed",
						  G_CALLBACK (applet_preferences_on_terminal_changed),
						  (gpointer)self);

		/***** show hidden *****/
		show_hidden = glade_xml_get_widget (xml, "show_hidden_check");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_hidden),
									  mb_prefs->browser_prefs->show_hidden);
		g_signal_connect (G_OBJECT (show_hidden),
						  "toggled",
						  G_CALLBACK (applet_preferences_on_show_hidden_pressed),
						  (gpointer)self);

		/***** icon *****/
		show_icon = glade_xml_get_widget (xml, "show_icon_check");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_icon),
									  mb_prefs->show_icon);
		g_signal_connect (G_OBJECT (show_icon),
						  "toggled",
						  G_CALLBACK (applet_preferences_on_show_icon_pressed),
						  (gpointer)self);

		icon_button = glade_xml_get_widget (xml, "icon_button");
		GtkWidget *icon = utils_get_scaled_image_from_file (mb_prefs->icon, ICON_BUTTON_SIZE);
		gtk_button_set_image (GTK_BUTTON (icon_button), icon);
		g_signal_connect (G_OBJECT (icon_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_icon_select),
						  (gpointer)self);

		/***** dirs/labels **/
		self->priv->tree_view = glade_xml_get_widget (xml, "directories_tree");
		applet_preferences_create_list_view (self);

		/***** more buttons ******/
		GtkWidget *add_button  = glade_xml_get_widget (xml, "directory_add_button");
		GtkWidget *rem_button  = glade_xml_get_widget (xml, "directory_remove_button");
		GtkWidget *up_button   = glade_xml_get_widget (xml, "move_up_button");
		GtkWidget *down_button = glade_xml_get_widget (xml, "move_down_button");


		g_signal_connect (G_OBJECT (add_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_add_dir_clicked),
						  (gpointer) self);
		g_signal_connect (G_OBJECT (rem_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_rem_dir_clicked),
						  (gpointer) self);
		g_signal_connect (G_OBJECT (up_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_up_dir_clicked),
						  (gpointer) self);
		g_signal_connect (G_OBJECT (down_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_down_dir_clicked),
						  (gpointer) self);

		/***** the end ******/
		self->priv->window = window;
	}

	gtk_widget_show_all (self->priv->window);
	gtk_window_present (GTK_WINDOW (self->priv->window));

	return;
}
/******************************************************************************/
static MenuBarPrefs *
applet_preferences_load_from_gconf (PanelApplet *applet) {
	GError *error = NULL;

	MenuBarPrefs *mb_prefs  = g_new0 (MenuBarPrefs, 1);
	mb_prefs->browser_prefs = g_new0 (BrowserPrefs, 1);

	/* this loads the default key's/values into the gconf entry for this applet
	 * instance. It also check to make sure the values were retrieved properly
	 * AND that they are valid */
	panel_applet_add_preferences (applet, "/schemas/apps/menu-file-browser-applet/prefs", &error);
	if (utils_check_gerror (&error)) return NULL;	

	/* show hidden files? */
	mb_prefs->browser_prefs->show_hidden = panel_applet_gconf_get_bool (applet,
																	 KEY_HIDDEN_SHOW,
																	 &error);
	if (utils_check_gerror (&error)) {
		mb_prefs->browser_prefs->show_hidden = DEFAULT_SHOW_HIDDEN;
		panel_applet_gconf_set_bool (applet,
									 KEY_HIDDEN_SHOW,
									 mb_prefs->browser_prefs->show_hidden,
									 &error);
	}
	/* terminal */
	mb_prefs->browser_prefs->terminal = panel_applet_gconf_get_string (applet,
																	KEY_TERMINAL,
																	&error);
	if (utils_check_gerror (&error) || mb_prefs->browser_prefs->terminal == NULL) {
		mb_prefs->browser_prefs->terminal = g_strdup (DEFAULT_TERMINAL);
		panel_applet_gconf_set_string (applet,
									   KEY_TERMINAL,
									   mb_prefs->browser_prefs->terminal,
									   &error);
	}
	/* the icon */
	mb_prefs->icon = panel_applet_gconf_get_string (applet,
												 KEY_ICON_NAME,
												 &error);
	if (utils_check_gerror (&error) || mb_prefs->icon == NULL) {
		mb_prefs->icon = g_strdup (DEFAULT_ICON);
		panel_applet_gconf_set_string (applet,
									   KEY_ICON_NAME,
									   mb_prefs->icon,
									   &error);
	}
	/* show the icon? */
	mb_prefs->show_icon = panel_applet_gconf_get_bool (applet,
													KEY_ICON_SHOW,
													&error);
	if (utils_check_gerror (&error)) {
		mb_prefs->show_icon = DEFAULT_SHOW_ICON;
		panel_applet_gconf_set_bool (applet,
									 KEY_ICON_SHOW,
									 mb_prefs->show_icon,
									 &error);
	}
	/* directory list */
	GSList *dirs = panel_applet_gconf_get_list (applet,
											   KEY_DIR,
											   GCONF_VALUE_STRING,
											   &error);
	if (utils_check_gerror (&error) || dirs == NULL) {
		dirs = g_slist_alloc ();
		dirs->data = g_strdup (DEFAULT_PATH);
		dirs->next = NULL;
		panel_applet_gconf_set_list (applet,
									 KEY_DIR,
									 GCONF_VALUE_STRING,
									 dirs,
									 &error);
	}
	/* labels list */
	GSList *labels = panel_applet_gconf_get_list (applet,
											     KEY_LABELS,
											     GCONF_VALUE_STRING,
											     &error);
	if (utils_check_gerror (&error) || labels == NULL) {
		labels = g_slist_alloc ();
		labels->data = g_strdup (DEFAULT_LABEL);
		labels->next = NULL;
		panel_applet_gconf_set_list (applet,
									 KEY_LABELS,
									 GCONF_VALUE_STRING,
									 labels,
									 &error);
	}
	mb_prefs->dirs   = dirs;
	mb_prefs->labels = labels;

	return mb_prefs;
}
/******************************************************************************/
AppletPreferences*
applet_preferences_new (PanelApplet* applet) {
	AppletPreferences *self;
	g_return_val_if_fail (applet == NULL || PANEL_IS_APPLET (applet), NULL);
	self = g_object_newv (TYPE_APPLET_PREFERENCES, 0, NULL);

	self->menu_bar_prefs = applet_preferences_load_from_gconf (applet);
	self->priv->window = NULL;
	self->priv->applet = applet;


	return self;
}
/******************************************************************************/
static void
applet_preferences_class_init (AppletPreferencesClass * klass) {
	applet_preferences_parent_class = g_type_class_peek_parent (klass);
	g_type_class_add_private (klass, sizeof (AppletPreferencesPrivate));
	G_OBJECT_CLASS (klass)->dispose = applet_preferences_dispose;

	applet_preferences_signals [PREFS_CHANGED] =
		g_signal_new ("prefs_changed",
    	              G_TYPE_FROM_CLASS (klass),
					  G_SIGNAL_NO_HOOKS,
            	      /*G_STRUCT_OFFSET (AppletPreferencesClass, prefs_changed),*/
					  0,
					  NULL,
					  NULL,
				      g_cclosure_marshal_VOID__INT,
					  G_TYPE_NONE,
					  1,
					  G_TYPE_POINTER);
	/* NOTE!!! a dynamically allocated PrefsChangedSignalData structure is passed
	 * to the callback function including 2 gchar pointers. The callback function
	 * is responsible for freeing this memory. HOWEVER!!! make sure that the gchar*
	 * can be freed!!! Be careful!!!*/
}
/******************************************************************************/
static void
applet_preferences_init (AppletPreferences * self) {
	self->priv = APPLET_PREFERENCES_GET_PRIVATE (self);
}
/******************************************************************************/
static void
applet_preferences_dispose (GObject * obj) {
	AppletPreferences * self;
	self = APPLET_PREFERENCES (obj);
	(self->priv->window == NULL ? NULL : (self->priv->window = (g_object_unref (self->priv->window), NULL)));
	G_OBJECT_CLASS (applet_preferences_parent_class)->dispose (obj);
}
/******************************************************************************/
GType
applet_preferences_get_type (void) {
	static GType applet_preferences_type_id = 0;
	if (G_UNLIKELY (applet_preferences_type_id == 0)) {
		static const GTypeInfo g_define_type_info = { sizeof (AppletPreferencesClass),
													  (GBaseInitFunc) NULL,
													  (GBaseFinalizeFunc) NULL,
													  (GClassInitFunc) applet_preferences_class_init,
													  (GClassFinalizeFunc) NULL,
													  NULL,
													  sizeof (AppletPreferences),
													  0,
													  (GInstanceInitFunc) applet_preferences_init };

		applet_preferences_type_id = g_type_register_static (G_TYPE_OBJECT,
															 "AppletPreferences",
															 &g_define_type_info, 0);
	}
	return applet_preferences_type_id;
}
/******************************************************************************/
