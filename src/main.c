/*
 * File:				main.c
 * Created:				September 2005
 * Created by:			Axel von Bertoldi
 * Last Modified:		March 2007
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

#define APPLET_IID			"OAFIID:GNOME_MenuFileBrowserApplet"
#define APPLET_FACTORY_IID	"OAFIID:GNOME_MenuFileBrowserApplet_Factory"
#define TOOLTIP_TEXT		"Browse and open files in you home directory"
#define VERSION				"0.3"

/******************************************************************************/
static void file_browser_applet_display_properties_dialog (GtkWidget *widget);
static void file_browser_applet_display_about_dialog (GtkWidget *widget);
static void file_browser_applet_display_help_dialog (GtkWidget *widget);
static void file_browser_applet_hide_tooltip (GtkWidget *widget, GtkWidget *menu_bar);
static void file_browser_applet_show_tooltip (GtkWidget *widget, GtkWidget *menu_bar);
static gboolean file_browser_applet_create (PanelApplet *applet);
static gboolean file_browser_applet_factory (PanelApplet *applet, const  gchar *iid, gpointer data);
/******************************************************************************/
GtkTooltips *tooltip = NULL;
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
static const BonoboUIVerb file_browser_applet_menu_verbs [] =
{
	BONOBO_UI_UNSAFE_VERB ("Preferences", file_browser_applet_display_properties_dialog),
	BONOBO_UI_UNSAFE_VERB ("Help",       file_browser_applet_display_help_dialog),
	BONOBO_UI_UNSAFE_VERB ("About",      file_browser_applet_display_about_dialog),
	BONOBO_UI_VERB_END
};
/******************************************************************************/
static void
file_browser_applet_display_properties_dialog (GtkWidget *widget)
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
                            					GTK_DIALOG_MODAL |
												GTK_DIALOG_DESTROY_WITH_PARENT |
												GTK_DIALOG_NO_SEPARATOR,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_CLOSE,
												"No preferences just yet...");
	gtk_window_set_title (GTK_WINDOW (dialog), "Preferences");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return;
}
/******************************************************************************/
static void
file_browser_applet_display_help_dialog (GtkWidget *widget)
{
	GtkWidget *dialog = gtk_message_dialog_new (NULL,
                            					GTK_DIALOG_MODAL |
												GTK_DIALOG_DESTROY_WITH_PARENT |
												GTK_DIALOG_NO_SEPARATOR,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_CLOSE,
												"Sorry, no help or documentation yet...");
	gtk_window_set_title (GTK_WINDOW (dialog), "Help");
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	return;
}
/******************************************************************************/
static void
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
		g_free(file);
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
	return;
}
/******************************************************************************/
static void
file_browser_applet_hide_tooltip (GtkWidget *widget,
								  GtkWidget *menu_bar)
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
								  GtkWidget *menu_bar)
{
	gtk_tooltips_enable (tooltip);
	return;
}
/******************************************************************************/
static gboolean
file_browser_applet_create (PanelApplet *applet)
{
	MenuFileBrowser *file_browser;
	GtkWidget *menu_bar;
	GtkWidget *icon;

	/* here or in the browser? */
    gnome_vfs_init();    
    
    /* Create a menu-bar to hold the menu items */
    menu_bar = gtk_menu_bar_new ();

	tooltip = gtk_tooltips_new ();
    
	gtk_tooltips_set_tip (tooltip,
						  GTK_WIDGET (menu_bar),	
						  _(TOOLTIP_TEXT),
						  NULL);

 	file_browser = menu_file_browser_new ("Home",
										  g_get_home_dir());

    /* get the icon and add it to the menu item */
	icon  = gtk_image_new_from_stock ("gtk-home",
									  GTK_ICON_SIZE_MENU);	
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_browser->menu_item),
                                    					icon);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar),
                           file_browser->menu_item);

	gtk_container_add (GTK_CONTAINER (applet),
					   menu_bar);	

	panel_applet_set_flags (applet,
							PANEL_APPLET_HAS_HANDLE);

	g_signal_connect (G_OBJECT (file_browser->menu_item),
					  "activate",
					  G_CALLBACK (file_browser_applet_hide_tooltip),
					  menu_bar);

	g_signal_connect (G_OBJECT (file_browser->menu_item),
                	  "deselect",
					  G_CALLBACK (file_browser_applet_show_tooltip),
					  menu_bar);

	panel_applet_setup_menu (PANEL_APPLET (applet),
							 file_browser_applet_menu_xml,
							 file_browser_applet_menu_verbs,
							 NULL);
	gtk_rc_parse_string (
		"style \"panel-menubar-style\"\n"
		"{\n"
		"  GtkMenuBar::shadow-type = none\n"
		"  GtkMenuBar::internal-padding = 0\n"
		"}\n"
		"class \"GtkMenuBar\" style \"panel-menubar-style\"");

	gtk_widget_show_all (GTK_WIDGET(applet));
	gtk_main();	
	
    return TRUE;
}
/******************************************************************************/
static gboolean
file_browser_applet_factory (PanelApplet *applet,
					const  gchar *iid,
					gpointer data)
{
        if(strcmp(iid, APPLET_IID) == 0)
        {
            return file_browser_applet_create(applet);
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
