/*
 * File:				preferences.c
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		October 2007
 * Last Modified by:	Axel von Bertoldi
 * (C) 2005,2006,2007	Axel von Bertoldi
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
#include <gtk/gtk.h>
#include <glib/gprintf.h>

/******************************************************************************/
struct _AppletPreferencesPrivate {
	GtkWidget	*window;
	PanelApplet *applet;
};
/******************************************************************************/
#define APPLET_PREFERENCES_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_APPLET_PREFERENCES, AppletPreferencesPrivate))
#define ICON_BUTTON_SIZE 45
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

/******************************************************************************/
static guint applet_preferences_signals[LAST_SIGNAL] = { 0 };
/* this is bad, but I need to get at the button from all over the file to
 * update it's state. The other option is make it private member. */
static GtkWidget *revert_button = NULL;
/******************************************************************************/
static gpointer applet_preferences_parent_class = NULL;
static void applet_preferences_dispose (GObject *obj);
/******************************************************************************/
static void
applet_preferences_on_show_icon_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *a_prefs = (AppletPreferences *)data;
	/* get the new state from the widget and update the prefs structure */
	a_prefs->menu_bar_prefs->show_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	/* emit the signal so the panel menu bar updates itself */
	g_signal_emit (G_OBJECT (data),
			applet_preferences_signals [PREFS_CHANGED],
			0,
			PREFS_SIGNAL_SHOW_ICON);

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	gtk_widget_set_sensitive (revert_button, TRUE);
	return;
}
/******************************************************************************/
static void
applet_preferences_on_show_hidden_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *a_prefs = (AppletPreferences *)data;
	/* get the new state from the widget and update the prefs structure. No
	 * need to let the menu bar or browser object know */
	a_prefs->menu_bar_prefs->browser_prefs->show_hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	gtk_widget_set_sensitive (revert_button, TRUE);
	return;
}
/******************************************************************************/
static void
applet_preferences_on_terminal_changed (GtkWidget *widget, gpointer data) {
	gchar *tmp = NULL;
	AppletPreferences *a_prefs = (AppletPreferences *)data;
	
	/* get the new state from the widget and update the prefs structure. No
	 * need to let the menu bar or browser object know */
	tmp = a_prefs->menu_bar_prefs->browser_prefs->terminal;
	g_free (tmp);
	a_prefs->menu_bar_prefs->browser_prefs->terminal = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

	/* update the state of the revert button. A pref has changed so the button
	 * should now be sensitive  */
	gtk_widget_set_sensitive (revert_button, TRUE);
	return;
}	
/******************************************************************************/
static void
applet_preferences_on_icon_select (GtkWidget *button, gpointer data) {
	GtkWidget *file_chooser_dialog;
	AppletPreferences *a_prefs = (AppletPreferences *)data;
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
								   a_prefs->menu_bar_prefs->icon);
	/* check the reply */
	if (gtk_dialog_run (GTK_DIALOG (file_chooser_dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *new_icon;
		new_icon = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser_dialog));
		g_printf ("the new icon is %s\n", new_icon);
		/* make sure the new icon is not the same as the old one */	
		if (g_ascii_strcasecmp (new_icon, a_prefs->menu_bar_prefs->icon)) {
			/* update the prefs structure */
			g_free (a_prefs->menu_bar_prefs->icon);
			a_prefs->menu_bar_prefs->icon = new_icon;

			/* update the button's icon */
			GtkWidget *icon = gtk_button_get_image (GTK_BUTTON (button));
			gtk_widget_destroy (icon);
			icon = utils_get_scaled_image_from_file (a_prefs->menu_bar_prefs->icon,
													 ICON_BUTTON_SIZE);
			gtk_button_set_image (GTK_BUTTON (button), icon);

			/* emit the signal so the panel menu bar updates itself */
			g_signal_emit (G_OBJECT (data),
						   applet_preferences_signals [PREFS_CHANGED],
						   0,
						   PREFS_SIGNAL_ICON_CHANGED);
		}
		else {
			/* they chose the same icon again! */
			g_free (new_icon);
		}

	}
	/* clean up and update the state of the revert button. A pref has changed
	 * so the button should now be sensitive  */
	gtk_widget_destroy (file_chooser_dialog);
	gtk_widget_set_sensitive (revert_button, TRUE);
}
/******************************************************************************/
static void
applet_preferences_save_to_gconf (AppletPreferences *a_prefs) {
	PanelApplet *applet = a_prefs->priv->applet;
	GError *error = NULL;
	/* save the data in the prefs structure to the gconf schema (or whatever)*/
	/* show hidden files? */
	panel_applet_gconf_set_bool (applet,
								 KEY_HIDDEN_SHOW,
								 a_prefs->menu_bar_prefs->browser_prefs->show_hidden,
								 &error);
	utils_check_gerror (&error);
	/* terminal */
	panel_applet_gconf_set_string (applet,
								   KEY_TERMINAL,
								   a_prefs->menu_bar_prefs->browser_prefs->terminal,
								   &error);
	utils_check_gerror (&error);
	/* the icon */
	panel_applet_gconf_set_string (applet,
								   KEY_ICON_NAME,
								   a_prefs->menu_bar_prefs->icon,
								   &error);
	utils_check_gerror (&error);
	/* show the icon? */
	panel_applet_gconf_set_bool (applet,
								 KEY_ICON_SHOW,
								 a_prefs->menu_bar_prefs->show_icon,
								 &error);
	utils_check_gerror (&error);
	/* directory list */
	/* labels list */
	return;
}
/******************************************************************************/
static void
applet_preferences_dialog_response (GtkWidget *window, gint response, gpointer data) {
	AppletPreferences *a_prefs = (AppletPreferences *)data;
	/* figure out what button closed the dialog and take the appropriate action */
	switch (response) {
		case RESPONSE_REVERT:
			/* revert to the saved config */
			gtk_widget_set_sensitive (revert_button, FALSE);
			break;

		case GTK_RESPONSE_CLOSE:
			/* save the current configuration */
			applet_preferences_save_to_gconf (a_prefs);
			gtk_widget_set_sensitive (revert_button, FALSE);
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
void
applet_preferences_make_dialog (AppletPreferences *applet_prefs) {
	g_return_if_fail (applet_prefs == NULL || IS_APPLET_PREFERENCES (applet_prefs));

	GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *show_icon;
    GtkWidget *icon_button;
    GtkWidget *show_hidden;
    GtkWidget *terminal;
	MenuBarPrefs *mb_prefs = applet_prefs->menu_bar_prefs;

	if (applet_prefs->priv->window == NULL) {
		window = gtk_dialog_new_with_buttons ("Menu File Browser Applet Preferences",
											  NULL,
											  0,
											  NULL);
		
		gtk_window_set_title (GTK_WINDOW (window), "Menu File Browser Applet Preferences");
		gtk_window_set_icon_name (GTK_WINDOW (window), "menu-file-browser-applet");
		gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

		revert_button = gtk_dialog_add_button (GTK_DIALOG (window),
											   GTK_STOCK_REVERT_TO_SAVED,
											   RESPONSE_REVERT);
		gtk_widget_set_sensitive (revert_button, FALSE);
		gtk_dialog_add_button (GTK_DIALOG (window),
							   GTK_STOCK_CLOSE,
							   GTK_RESPONSE_CLOSE);

		g_signal_connect (G_OBJECT (GTK_DIALOG (window)),
						  "response",
						  G_CALLBACK (applet_preferences_dialog_response),
						  (gpointer) applet_prefs);

		g_signal_connect (G_OBJECT (window),
						  "delete_event",
						  G_CALLBACK (gtk_widget_hide),
						  (gpointer) applet_prefs);

		/***** terminal *****/
		hbox = gtk_hbox_new (FALSE, 0);
		terminal = gtk_entry_new ();
		gtk_entry_set_width_chars (GTK_ENTRY (terminal), 15);
		gtk_entry_set_text (GTK_ENTRY (terminal),
							mb_prefs->browser_prefs->terminal);
		g_signal_connect (G_OBJECT (terminal),			
						  "changed",
						  G_CALLBACK (applet_preferences_on_terminal_changed),
						  (gpointer)applet_prefs);
		gtk_container_add (GTK_CONTAINER (hbox), gtk_label_new (" terminal "));
		gtk_container_add (GTK_CONTAINER (hbox), terminal);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), hbox);

		/***** show hidden *****/
		show_hidden = gtk_check_button_new_with_label ("show hidden");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_hidden),
									  mb_prefs->browser_prefs->show_hidden);
		g_signal_connect (G_OBJECT (show_hidden),
						  "toggled",
						  G_CALLBACK (applet_preferences_on_show_hidden_pressed),
						  (gpointer)applet_prefs);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), show_hidden);

		/***** icon *****/
		hbox = gtk_hbox_new (FALSE, 0);

		show_icon = gtk_check_button_new_with_label ("show icon");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_icon),
									  mb_prefs->show_icon);
		g_signal_connect (G_OBJECT (show_icon),
						  "toggled",
						  G_CALLBACK (applet_preferences_on_show_icon_pressed),
						  (gpointer)applet_prefs);

		icon_button = gtk_button_new ();
		GtkWidget *icon = utils_get_scaled_image_from_file (mb_prefs->icon, ICON_BUTTON_SIZE);
		gtk_button_set_image (GTK_BUTTON (icon_button), icon);
		g_signal_connect (G_OBJECT (icon_button),
						  "released",
						  G_CALLBACK (applet_preferences_on_icon_select),
						  (gpointer)applet_prefs);

		gtk_container_add (GTK_CONTAINER (hbox), show_icon);
		gtk_container_add (GTK_CONTAINER (hbox), icon_button);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), hbox);

		/***** other ********/

		applet_prefs->priv->window = window;

	}

	gtk_widget_show_all (applet_prefs->priv->window);
	gtk_window_present (GTK_WINDOW (applet_prefs->priv->window));

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
	/*===*/
	mb_prefs->dirs = g_ptr_array_new();
	mb_prefs->labels = g_ptr_array_new();
	GSList *dirs_head=dirs;
	GSList *labels_head=labels;

	while (dirs && labels) {
		g_printf ("%s:%s\n", (gchar*)labels->data, (gchar*)dirs->data);
		g_ptr_array_add (mb_prefs->dirs, (gchar*)dirs->data);
		g_ptr_array_add (mb_prefs->labels, (gchar*)labels->data);
		dirs = dirs->next;
		labels = labels->next;
	}
	g_slist_free (dirs_head);
	g_slist_free (labels_head);

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
					  G_TYPE_INT);
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
