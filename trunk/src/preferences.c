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

/******************************************************************************/
BrowserPreferences	*
preferences_browser_get_default () {
	BrowserPreferences *prefs = g_new0 (BrowserPreferences, 1);

	prefs->show_hidden = DEFAULT_SHOW_HIDDEN;
	prefs->terminal = g_strdup (DEFAULT_TERMINAL);
	return prefs;	
}
/******************************************************************************/
AppletPreferences *
preferences_load_from_gconf (PanelApplet *applet) {
	GError *error = NULL;

	AppletPreferences *prefs = g_new0 (AppletPreferences, 1);
	prefs->browser_prefs = g_new0 (BrowserPreferences, 1);

	/* this loads the default key's/values into the gconf entry for this applet instance*/
	panel_applet_add_preferences (applet, "/schemas/apps/menu-file-browser-applet/prefs", &error);
	if (utils_check_gerror (&error)) return NULL;	

	/*===*/
	prefs->browser_prefs->show_hidden = panel_applet_gconf_get_bool (applet,
																	 KEY_HIDDEN_SHOW,
																	 &error);
	if (utils_check_gerror (&error)) {
		prefs->browser_prefs->show_hidden = DEFAULT_SHOW_HIDDEN;
		panel_applet_gconf_set_bool (applet,
									 KEY_HIDDEN_SHOW,
									 prefs->browser_prefs->show_hidden,
									 &error);
	}
	/*===*/
	prefs->browser_prefs->terminal = panel_applet_gconf_get_string (applet,
																	KEY_TERMINAL,
																	&error);
	if (utils_check_gerror (&error) || prefs->browser_prefs->terminal == NULL) {
		prefs->browser_prefs->terminal = g_strdup (DEFAULT_TERMINAL);
		panel_applet_gconf_set_string (applet,
									   KEY_TERMINAL,
									   prefs->browser_prefs->terminal,
									   &error);
	}
	/*===*/
	prefs->icon = panel_applet_gconf_get_string (applet,
												 KEY_ICON_NAME,
												 &error);
	if (utils_check_gerror (&error) || prefs->icon == NULL) {
		prefs->icon = g_strdup (DEFAULT_ICON);
		panel_applet_gconf_set_string (applet,
									   KEY_ICON_NAME,
									   prefs->icon,
									   &error);
	}
	/*===*/
	prefs->show_icon = panel_applet_gconf_get_bool (applet,
													KEY_ICON_SHOW,
													&error);
	if (utils_check_gerror (&error)) {
		prefs->show_icon = DEFAULT_SHOW_ICON;
		panel_applet_gconf_set_bool (applet,
									 KEY_ICON_SHOW,
									 prefs->show_icon,
									 &error);
	}
	/*===*/
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
	/*===*/
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
	prefs->dirs = g_ptr_array_new();
	prefs->labels = g_ptr_array_new();
	GSList *dirs_head=dirs;
	GSList *labels_head=labels;

	while (dirs && labels) {
		g_printf ("%s:%s\n", (gchar*)labels->data, (gchar*)dirs->data);
		g_ptr_array_add (prefs->dirs, (gchar*)dirs->data);
		g_ptr_array_add (prefs->labels, (gchar*)labels->data);
		dirs = dirs->next;
		labels = labels->next;
	}
	g_slist_free (dirs_head);
	g_slist_free (labels_head);

	return prefs;
}
/******************************************************************************/
AppletPreferences *
preferences_get (PanelApplet *applet) {
	AppletPreferences *prefs = NULL;
	
	prefs = preferences_load_from_gconf (applet);
	return prefs;	
}
/******************************************************************************/
/******************************************************************************/
gboolean
on_show_icon_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *prefs = (AppletPreferences *)data;

	prefs->show_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	return TRUE;
}
/******************************************************************************/
gboolean
on_show_hidden_pressed (GtkWidget *widget, gpointer data) {
	AppletPreferences *prefs = (AppletPreferences *)data;

	prefs->browser_prefs->show_hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	return TRUE;
}
/******************************************************************************/
gboolean
on_terminal_changed (GtkWidget *widget, gpointer data) {
	gchar *tmp = NULL;
	AppletPreferences *prefs = (AppletPreferences *)data;
	
	tmp = prefs->browser_prefs->terminal;
	g_free (tmp);
	prefs->browser_prefs->terminal = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

	return FALSE;
}	
/******************************************************************************/
void
preferences_make_window (AppletPreferences *prefs) {
	GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *show_icon;
    GtkWidget *show_hidden;
    GtkWidget *terminal;

    window = gtk_dialog_new_with_buttons ("Menu File Browser Applet Preferences",
										  NULL,
										  GTK_DIALOG_NO_SEPARATOR,
										  GTK_STOCK_CLOSE,
										  GTK_RESPONSE_CLOSE,
										  NULL);

    gtk_window_set_title (GTK_WINDOW (window), "Menu File Browser Applet Preferences");
	g_signal_connect (G_OBJECT (window), "destroy",
					  G_CALLBACK (gtk_widget_destroy), window);
    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (gtk_widget_destroy), window);

	g_signal_connect_swapped (G_OBJECT (window),
							  "response", 
							  G_CALLBACK (gtk_widget_destroy),
							  window);

	vbox  = gtk_vbox_new (TRUE, 0);

	/***** icon *****/
	hbox = gtk_hbox_new (FALSE, 0);

	show_icon = gtk_check_button_new_with_label ("show icon");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_icon),
								  prefs->show_icon);
	g_signal_connect (G_OBJECT (show_icon),
					  "toggled",
					  G_CALLBACK (on_show_icon_pressed),
					  (gpointer)prefs);
	gtk_container_add (GTK_CONTAINER (hbox), 
					   gtk_button_new_with_label ("Icon"));
	gtk_container_add (GTK_CONTAINER (hbox), show_icon);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), hbox);

	/***** show hidden *****/
	show_hidden = gtk_check_button_new_with_label ("show hidden");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_hidden),
								  prefs->browser_prefs->show_hidden);
	g_signal_connect (G_OBJECT (show_hidden),
					  "toggled",
					  G_CALLBACK (on_show_hidden_pressed),
					  (gpointer)prefs);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), show_hidden);

	/***** terminal *****/
	hbox = gtk_hbox_new (FALSE, 0);
	terminal = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (terminal), 15);
	gtk_entry_set_text (GTK_ENTRY (terminal),
						prefs->browser_prefs->terminal);
	g_signal_connect (G_OBJECT (terminal),			
					  "changed",
					  G_CALLBACK (on_terminal_changed),
					  (gpointer)prefs);
	gtk_container_add (GTK_CONTAINER (hbox), gtk_label_new (" terminal "));
	gtk_container_add (GTK_CONTAINER (hbox), terminal);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), hbox);


	/*gtk_container_add (GTK_CONTAINER (window), vbox);*/
	gtk_widget_show_all (window);
	return;
}
/******************************************************************************/
