/*
 * File:				preferences.c
 * Created:				April 2006
 * Created by:			Axel von Bertoldi
 * Last Modified:		April 2007
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
void
preferences_delete (AppletPreferences *prefs)
{
	int i;

	for (i=0; i < prefs->dirs->len; i++)
	{
		g_free ( (gchar*)g_ptr_array_index (prefs->dirs, i));
		g_free ( (gchar*)g_ptr_array_index (prefs->labels, i));
	}
	g_ptr_array_free (prefs->dirs, TRUE);
	g_ptr_array_free (prefs->labels, TRUE);

	g_free (prefs->icon);
	g_free (prefs->browser_prefs);
	g_free (prefs);
}
/******************************************************************************/
/* This creates a default .mfbsrc file if one does not exist
 * It could probably be done a lot better... */
gboolean
preferences_save_new (AppletPreferences *prefs)
{
	GError *error = NULL;
	gchar *rc_file = NULL;
	gchar *contents = NULL;
	gchar *tmp = NULL;
	gint i;

	rc_file = g_strdup_printf ("%s/%s",
							   g_get_home_dir(),
							   MFBARC);

	contents = g_strdup_printf ("[%s]\n", DIR_GROUP);

	for (i=0; i < prefs->dirs->len; i++)
	{
		tmp = contents;
		contents = g_strdup_printf ("%s%s%d=%s;%s\n",
									contents,
									KEY_DIR,
									i,
									(gchar*)g_ptr_array_index (prefs->dirs, i),
									(gchar*)g_ptr_array_index (prefs->labels, i));
		g_free (tmp);
	}

	tmp = contents;
	contents = g_strdup_printf ("%s\n[%s]\n",
								contents,
								PREF_GROUP);
	g_free (tmp);

	tmp = contents;
	contents = g_strdup_printf ("%s"
								"%s=%s\n"
								"%s=%d\n"
								"%s=%d\n"
								"%s=%s\n",
								contents,
								KEY_ICON_NAME, prefs->icon,
								KEY_ICON_SHOW, prefs->show_icon,
								KEY_HIDDEN_SHOW, prefs->browser_prefs->show_hidden,
								KEY_TERMINAL, prefs->browser_prefs->terminal);
	g_free (tmp);

/* FIXME: finish the config dialog and get rid of this junk */
tmp = contents;
contents = g_strdup_printf ("%s"
							"\n#do a 'killall menu-file-browser-applet' to refresh (for now)",
							contents);
g_free (tmp);
/* FIXME: end*/

	g_file_set_contents (rc_file,
						 contents,
						 -1,
						 &error);
	
	if (utils_check_gerror (&error)) return FALSE;

	g_free (rc_file);
	g_free (contents);

	return TRUE;
}
/******************************************************************************/
/* Clearly this does shit-all atm, but is supposed to save the applet config
 * data to the file keeping the original formatting (like comments and stuff).
 * Is keeping formating worth the effort? */
gboolean
preferences_save (AppletPreferences *prefs)
{
	return TRUE;
}
/******************************************************************************/
/* create default config options */
AppletPreferences *
preferences_create_defaults ()
{
	AppletPreferences *prefs = g_new0 (AppletPreferences, 1);
	prefs->browser_prefs		 = menu_browser_get_default_prefs ();

	prefs->dirs = g_ptr_array_new();
	g_ptr_array_add (prefs->dirs, (gchar*)DEFAULT_PATH);

	prefs->labels = g_ptr_array_new();
	g_ptr_array_add (prefs->labels, DEFAULT_LABEL);
	prefs->icon = g_strdup (DEFAULT_ICON);
	prefs->show_icon = DEFAULT_SHOW_ICON;

	return prefs;	
}
/******************************************************************************/
AppletPreferences *
preferences_process_options (AppletPreferences *prefs, GKeyFile *key_file)
{
	gint k,v;
	GError *error = NULL;
	gchar **keys = NULL;
	gchar **values = NULL;

	keys = g_key_file_get_keys (key_file,
								PREF_GROUP,
								NULL,
								&error);
	if (utils_check_gerror (&error)) return prefs;

	for (k = 0; keys[k]; k++)
	{
		values = g_key_file_get_string_list (key_file,
											 PREF_GROUP,
											 keys[k],
											 NULL,
											 &error);
		if (utils_check_gerror (&error)) return prefs;

		for (v = 0; values[v]; v++)
		{
			if (g_ascii_strcasecmp (keys[k], KEY_ICON_NAME) == 0)
			{

    			if (!g_file_test (values[v], G_FILE_TEST_EXISTS))
				{
					gchar *message = g_strdup_printf ("Error: icon file %s does not exist.", values[v]);
					g_printf ("%s\n", message);

					utils_show_dialog ("Error: File does not exist",
									   message,
									   GTK_MESSAGE_ERROR);
					g_free (message);
				}
				else
				{
					g_free (prefs->icon);
					prefs->icon = g_strdup (values[v]);
				}
			}
			else if (g_ascii_strcasecmp (keys[k], KEY_ICON_SHOW) == 0)
			{
				prefs->show_icon = g_key_file_get_boolean (key_file,
														 PREF_GROUP,
														 keys[k],
														 &error);
			}
			else if (g_ascii_strcasecmp (keys[k], KEY_HIDDEN_SHOW) == 0)
			{
				prefs->browser_prefs->show_hidden = g_key_file_get_boolean (key_file,
																		PREF_GROUP,
																		keys[k],
																		&error);
			}
			else if (g_ascii_strcasecmp (keys[k], KEY_TERMINAL) == 0)
			{
				g_free (prefs->browser_prefs->terminal);
				prefs->browser_prefs->terminal = g_strdup (values[v]);
			}
		}
		g_strfreev (values);
	}
	g_strfreev (keys);
	return prefs;
}
/******************************************************************************/
AppletPreferences *
preferences_process_dirs (AppletPreferences *prefs, GKeyFile *key_file)
{
	gint k,v;
	GError *error = NULL;
	gchar **keys = NULL;
	gchar **values = NULL;
	gboolean recreate = TRUE;

	keys = g_key_file_get_keys (key_file,
								DIR_GROUP,
								NULL,
								&error);
	if (utils_check_gerror (&error)) return prefs;

	for (k = 0; keys[k]; k++)
	{
		values = g_key_file_get_string_list (key_file,
											 DIR_GROUP,
											 keys[k],
											 NULL,
											 &error);
		if (utils_check_gerror (&error)) return prefs;
		for (v = 0; values[v]; v++)
		{
   			if (g_str_has_prefix (keys[k], KEY_DIR))
		    {
    			if (!g_file_test (values[v], G_FILE_TEST_IS_DIR))
				{
					gchar *message = g_strdup_printf ("Error: %s does not exist or is not a directory. Skipping...", values[v]);
					g_printf ("%s\n", message);

					utils_show_dialog ("Error: File does not exist",
									   message,
									   GTK_MESSAGE_ERROR);
					g_free (message);
					v++;
				}
				else
				{
					if (recreate)
					{	/* discard defaults if there is at least 1 key that starts with dir*/
						g_ptr_array_free (prefs->dirs, TRUE);
						g_ptr_array_free (prefs->labels, TRUE);
						prefs->dirs   = g_ptr_array_new();
						prefs->labels = g_ptr_array_new();
						recreate = FALSE;
					}
					g_ptr_array_add (prefs->dirs, g_strdup (values[v]));
					v++;
					g_ptr_array_add (prefs->labels, g_strdup (values[v]));
				}
			}
		}
		g_strfreev (values);
	}
	g_strfreev (keys);
	return prefs;
}
/******************************************************************************/
/* Get options from a file. Starts with default options and overrides them
 * whenever that option is also specified in the .mfbsrc file. If no .mfbsrc
 * file, exists, create a new one with the default options. This whole
 * approach needs rethinking... */
AppletPreferences *
preferences_load_from_file ()
{
	GKeyFile *key_file;
	GError *error = NULL;
	gchar *rc_file = NULL;
	gchar **groups = NULL;
	gint g;
	AppletPreferences *prefs = NULL;

	rc_file = g_strdup_printf ("%s/%s",
							   g_get_home_dir(),
							   MFBARC);

	/* create default options */
	prefs = preferences_create_defaults ();

	/* does the config file exist? */
    if (!g_file_test (rc_file, G_FILE_TEST_EXISTS))
	{	/* no. make a new one. */
		preferences_save_new (prefs);
		menu_browser_open_file (rc_file, EXEC_OPEN);
		return prefs;
	}
	else
	{
		/* yes. Read and parse it */
		key_file = g_key_file_new ();
		g_key_file_load_from_file (key_file,
								   rc_file,
								   G_KEY_FILE_KEEP_COMMENTS ||
								   G_KEY_FILE_KEEP_TRANSLATIONS,
								   &error);
		if (utils_check_gerror (&error)) return prefs;

		/* get the groups */
		groups = g_key_file_get_groups (key_file,
									NULL);
		for (g = 0; groups[g]; g++)
		{
			/******* Process Dirs group *****************/			
			if (g_ascii_strcasecmp (groups[g], DIR_GROUP) == 0)
			{
				prefs = preferences_process_dirs (prefs, key_file);
			}
			/******* Process Options group *****************/		
			if (g_ascii_strcasecmp (groups[g], PREF_GROUP) == 0)
			{
				prefs = preferences_process_options (prefs, key_file);
			}
		/************* Done processing Options group ***********/			
		}
	}
	g_strfreev (groups);
	g_key_file_free (key_file);

	return prefs;
}
/******************************************************************************/
AppletPreferences *
preferences_get ()
{
	AppletPreferences *prefs = NULL;

	prefs = preferences_load_from_file ();
	return prefs;	
}
/******************************************************************************/
/******************************************************************************/
gboolean
on_show_icon_pressed (GtkWidget *widget, gpointer data)
{
	AppletPreferences *prefs = (AppletPreferences *)data;

	prefs->show_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	return TRUE;
}
/******************************************************************************/
gboolean
on_show_hidden_pressed (GtkWidget *widget, gpointer data)
{
	AppletPreferences *prefs = (AppletPreferences *)data;

	prefs->browser_prefs->show_hidden = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	return TRUE;
}
/******************************************************************************/
gboolean
on_terminal_changed (GtkWidget *widget, gpointer data)
{
	gchar *tmp = NULL;
	AppletPreferences *prefs = (AppletPreferences *)data;
	
	tmp = prefs->browser_prefs->terminal;
	g_free (tmp);
	prefs->browser_prefs->terminal = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

	return FALSE;
}	
/******************************************************************************/
void
preferences_make_window (AppletPreferences *prefs)
{
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
