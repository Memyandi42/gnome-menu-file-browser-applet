/*
 * File:				main.c
 * Created:				September 2005
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

#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#include <panel-applet.h>

#include "panel-menu-bar.h"
#include "utils.h"

#define APPLET_IID			"OAFIID:GNOME_MenuFileBrowserApplet"
#define APPLET_FACTORY_IID	"OAFIID:GNOME_MenuFileBrowserApplet_Factory"
#define VERSION				"0.5.4"

/******************************************************************************/
static const gchar file_browser_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Preferences Item\" verb=\"Preferences\" _label=\"Preferences\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Help Item\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"About Item\" verb=\"About\" _label=\"About\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";
/******************************************************************************/
static gboolean
file_browser_applet_display_properties_dialog (GtkWidget *widget, gpointer *data) {

/*
	utils_show_dialog ("Preferences",
					   "Use Gconf for now...",
					   GTK_MESSAGE_ERROR);
*/

	PanelMenuBar *menu_bar = (PanelMenuBar *)data; 
	panel_menu_bar_edit_prefs (menu_bar);
	return FALSE;
}
/******************************************************************************/
static gboolean 
file_browser_applet_display_help_dialog (GtkWidget *widget) {

	if (1) {
	GError *error = NULL;

	gnome_help_display_desktop_on_screen (
	      NULL,
	      "menu-file-browser-applet", 
	      "menu-file-browser-applet", 
	      NULL, 
	      gtk_widget_get_screen (widget),
	      &error);

	if (error) {       
		utils_show_dialog ("Error",
						   "Could not display help.",
						   GTK_MESSAGE_ERROR);
		g_error_free (error);
	}
	}
	else {
	utils_show_dialog ("Help",
					   "Sorry, no help or documentation yet...",
					   GTK_MESSAGE_ERROR);
	}
	return FALSE;
}
/******************************************************************************/
static gboolean
file_browser_applet_display_about_dialog (GtkWidget *widget) {

	GdkPixbuf  *pixbuf = NULL;
	gchar       *file;
	const gchar *authors[] = {
		"Axel von Bertoldi <bertoldia@gmail.com>",
		"",
		"Contributions by:",
		"Stefano Maggiolo <maggiolo@mail.dm.unipi.it>",
		"Ivan N. Zlatev <contact@i-nz.net>",
		NULL
	};
	const gchar *documenters [] = {
/*		"Axel von Bertoldi", */
		NULL
	};
	const gchar *translator_credits = _("translator_credits");
	
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "menu-file-browser-applet.png", TRUE, NULL);
	if (file != NULL) {
		pixbuf = gdk_pixbuf_new_from_file (file, NULL);
		g_free (file);
	}

	gtk_show_about_dialog (NULL,
			       "name", _("Menu File Browser Applet"),
			       "version", VERSION,
			       "copyright", "Copyright \xc2\xa9 2006-2008 Axel von Bertoldi.",
			       "comments", _("Browse and open files in your home directory from the panel"),
			       "authors", authors,
			       "documenters", documenters,
			       "translator-credits", strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			       "logo", pixbuf,
			       NULL);

	if (pixbuf) {
		g_object_unref (pixbuf);
	}
	return 0;
}
/******************************************************************************/
static const
BonoboUIVerb file_browser_applet_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Preferences", file_browser_applet_display_properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("Help",        file_browser_applet_display_help_dialog),
	BONOBO_UI_UNSAFE_VERB ("About",       file_browser_applet_display_about_dialog),
	BONOBO_UI_VERB_END
};
/******************************************************************************/
static gboolean
file_browser_applet_create (PanelApplet *applet) {
    gnome_vfs_init ();

	PanelMenuBar* panel_menu_bar = panel_menu_bar_new (applet);

	panel_applet_set_flags (applet,
/*							PANEL_APPLET_EXPAND_MAJOR |*/
							PANEL_APPLET_EXPAND_MINOR |
							PANEL_APPLET_HAS_HANDLE);

	panel_applet_setup_menu (PANEL_APPLET (applet),
							 file_browser_applet_menu_xml,
							 file_browser_applet_menu_verbs,
							 (gpointer) panel_menu_bar);

	gtk_widget_show_all (GTK_WIDGET(applet));
	gtk_main ();	
	
    return TRUE;
}
/******************************************************************************/
static gboolean
file_browser_applet_factory (PanelApplet *applet,
							 const  gchar *iid,
							 gpointer data) {

        if (strcmp (iid, APPLET_IID) == 0) {
            return file_browser_applet_create (applet);
        }

        return FALSE;
}
/******************************************************************************/
PANEL_APPLET_BONOBO_FACTORY (APPLET_FACTORY_IID,
			     			PANEL_TYPE_APPLET,
							"file-browser-applet",
							"0",
							file_browser_applet_factory, 
							NULL)
/******************************************************************************/
