/*
 * File:				main.c
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		August 2007
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

#include <gtk/gtk.h>
#include <panel-applet.h>
#include "menu-browser.h"
#include "preferences.h"

#define APPLET_IID			"OAFIID:GNOME_MenuFileBrowserApplet"
#define APPLET_FACTORY_IID	"OAFIID:GNOME_MenuFileBrowserApplet_Factory"
#define TOOLTIP_TEXT		"Browse and open files in you home directory"
#define VERSION				"0.5.1"
#define ICON_SIZE			22 /* 22x22 pixels looks about right... */

/******************************************************************************/
typedef struct _FileBrowserApplet			FileBrowserApplet;
/******************************************************************************/
struct _FileBrowserApplet
{
	PanelApplet *applet;
	GtkWidget *menu_bar;
	AppletPreferences *prefs;

	GPtrArray *file_browser;
	GtkWidget *icon;
};
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
file_browser_applet_display_properties_dialog (GtkWidget *widget, gpointer *data)
{
/*
	utils_show_dialog ("Preferences",
					   "No preferences just yet...",
					   GTK_MESSAGE_ERROR);
*/

	gchar *rc_file = g_strdup_printf ("%s/%s",
									  g_get_home_dir (),
									  MFBARC);
	menu_browser_open_file (rc_file, EXEC_OPEN);

/*
	FileBrowserApplet *browser_applet = (FileBrowserApplet *) data;
	preferences_make_window (browser_applet->prefs);
*/
	return 0;
}
/******************************************************************************/
static gboolean 
file_browser_applet_display_help_dialog (GtkWidget *widget)
{

	utils_show_dialog ("Help",
					   "Sorry, no help or documentation yet...",
					   GTK_MESSAGE_ERROR);

	return 0;
}
/******************************************************************************/
static gboolean
file_browser_applet_display_about_dialog (GtkWidget *widget)
{
	GdkPixbuf  *pixbuf = NULL;
	gchar       *file;
	const gchar *authors[] =
	{
		"Axel von Bertoldi <bertoldia@gmail.com>",
		NULL
	};
	const gchar *documenters [] =
	{
/*		"Axel von Bertoldi", */
		NULL,
		NULL
	};
	const gchar *translator_credits = _("translator_credits");
	
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "menu-file-browser-applet.png", TRUE, NULL);
	if (file != NULL)
	{
		pixbuf = gdk_pixbuf_new_from_file (file, NULL);
		g_free (file);
	}

	gtk_show_about_dialog (NULL,
			       "name", _("Menu File Browser Applet"),
			       "version", VERSION,
			       "copyright", "Copyright \xc2\xa9 2006-2007 Axel von Bertoldi.",
			       "comments", _("Browse and open files in your home directory from the panel"),
			       "authors", authors,
			       "documenters", documenters,
			       "translator-credits", strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			       "logo", pixbuf,
			       NULL);

	if (pixbuf)
	{
		g_object_unref (pixbuf);
	}
	return 0;
}
/******************************************************************************/
static const
BonoboUIVerb file_browser_applet_menu_verbs [] =
{
	BONOBO_UI_UNSAFE_VERB ("Preferences", file_browser_applet_display_properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("Help",        file_browser_applet_display_help_dialog),
	BONOBO_UI_UNSAFE_VERB ("About",       file_browser_applet_display_about_dialog),
	BONOBO_UI_VERB_END
};
/******************************************************************************/
static void
file_browser_applet_hide_tooltip (GtkWidget *widget,
								  GtkTooltips *tooltip)
{
	gtk_tooltips_disable (tooltip);
	/* reposition the menu too. */
	GtkWidget *menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (widget));
	gtk_menu_reposition (GTK_MENU (menu));
	return;
}
/******************************************************************************/
static void
file_browser_applet_show_tooltip (GtkWidget *widget,
								  GtkTooltips *tooltip)
{
	gtk_tooltips_enable (tooltip);
	return;
}
/******************************************************************************/
void
file_browser_applet_delete (FileBrowserApplet *browser_applet)
{
	int i;
	MenuFileBrowser *tmp_file_browser = NULL;

	for (i=0; i < browser_applet->file_browser->len; i++)
	{
		tmp_file_browser = (MenuFileBrowser *)(g_ptr_array_index (browser_applet->file_browser, i));
		menu_browser_delete (tmp_file_browser);
	}
	g_ptr_array_free (browser_applet->file_browser, TRUE);

	gtk_widget_destroy (browser_applet->menu_bar);
	gtk_widget_destroy (browser_applet->icon);
}
/******************************************************************************
gboolean
file_browser_applet_reload (FileBrowserApplet *browser_applet)
{
	PanelApplet *applet = browser_applet->applet;

	file_browser_applet_delete (browser_applet);
    
	return file_browser_applet_create (applet);

}
******************************************************************************/
static gboolean
file_browser_applet_create (PanelApplet *applet)
{
	FileBrowserApplet *browser_applet = NULL;
	MenuFileBrowser *tmp_file_browser = NULL;
	gint i;
	GtkTooltips *tooltip = NULL;

    gnome_vfs_init ();

	browser_applet = g_new0(FileBrowserApplet, 1);
	browser_applet->file_browser = g_ptr_array_new ();
	
	/* read the configuration file, set the default values first*/
	browser_applet->prefs = preferences_get ();

    /* Create a menu-bar to hold the menu items */
    browser_applet->menu_bar = gtk_menu_bar_new ();

	tooltip = gtk_tooltips_new ();
	gtk_tooltips_set_tip (tooltip,
						  GTK_WIDGET (browser_applet->menu_bar),
						  _(TOOLTIP_TEXT),
						  NULL);

	/* for each path in the config file, make a browser object*/
	for (i=0; i < browser_applet->prefs->dirs->len; i++)
	{	/* make it */
		tmp_file_browser = menu_browser_new ((gchar*)(g_ptr_array_index (browser_applet->prefs->labels, i)),
				  								  (gchar*)(g_ptr_array_index (browser_applet->prefs->dirs, i)),
												  browser_applet->prefs->browser_prefs);
		/* add it to the list and to the menu bar*/
		g_ptr_array_add (browser_applet->file_browser, tmp_file_browser);

		gtk_menu_shell_append (GTK_MENU_SHELL (browser_applet->menu_bar),
							   tmp_file_browser->menu_item);
		/* attach the signal to hide the tooltips when selected*/
		g_signal_connect (G_OBJECT (tmp_file_browser->menu_item),
						  "activate",
						  G_CALLBACK (file_browser_applet_hide_tooltip),
						  tooltip);
	}

    /* add the image to the menu item */
	if (browser_applet->prefs->show_icon)
	{
		tmp_file_browser = (MenuFileBrowser *)(g_ptr_array_index (browser_applet->file_browser, 0));

		GdkPixbuf *orig   = gdk_pixbuf_new_from_file (browser_applet->prefs->icon ,NULL);
		/* FIXME look at gnome menu for icon size*/
		GdkPixbuf *scaled = gdk_pixbuf_scale_simple (orig,
                                          			 ICON_SIZE,
										  			 ICON_SIZE,
										  			 GDK_INTERP_HYPER);
		browser_applet->icon = gtk_image_new_from_pixbuf (scaled);
	    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tmp_file_browser->menu_item),
    	                                					browser_applet->icon);
		g_object_unref (orig);
		g_object_unref (scaled);
	}

	gtk_container_add (GTK_CONTAINER (applet),
					   browser_applet->menu_bar);	

	panel_applet_set_flags (applet,
/*							PANEL_APPLET_EXPAND_MAJOR ||
							PANEL_APPLET_EXPAND_MINOR ||*/
							PANEL_APPLET_HAS_HANDLE);


    g_signal_connect (G_OBJECT (browser_applet->menu_bar),
                	  "deactivate",
					  G_CALLBACK (file_browser_applet_show_tooltip),
					  tooltip);

	panel_applet_setup_menu (PANEL_APPLET (applet),
							 file_browser_applet_menu_xml,
							 file_browser_applet_menu_verbs,
							 browser_applet);
	gtk_rc_parse_string (
		"style \"panel-menubar-style\"\n"
		"{\n"
		"  GtkMenuBar::shadow-type = none\n"
		"  GtkMenuBar::internal-padding = 0\n"
		"}\n"
		"class \"GtkMenuBar\" style \"panel-menubar-style\"");

	gtk_widget_show_all (GTK_WIDGET(applet));
	gtk_main ();	
	
    return TRUE;
}
/******************************************************************************/
static gboolean
file_browser_applet_factory (PanelApplet *applet,
							 const  gchar *iid,
							 gpointer data)
{
        if (strcmp (iid, APPLET_IID) == 0)
        {
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
